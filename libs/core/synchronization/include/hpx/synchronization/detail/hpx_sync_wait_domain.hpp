//  Copyright (c) 2025 The HPX Authors
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/assert.hpp>
#include <hpx/modules/execution_base.hpp>
#include <hpx/synchronization/condition_variable.hpp>
#include <hpx/synchronization/spinlock.hpp>

#include <exception>
#include <mutex>
#include <optional>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

// HPX-aware sync_wait domain.
//
// stdexec::sync_wait normally blocks using OS condition variables via an
// internal run_loop. When called on an HPX worker thread, this can
// deadlock if completion requires more work on the same HPX thread pool,
// especially with --hpx:threads=1.
//
// hpx_sync_wait_domain customizes apply_sender(sync_wait_t, sndr) to use
// HPX-aware waiting with hpx::spinlock +
// hpx::condition_variable_any. This suspends the HPX task
// cooperatively instead of OS-blocking the worker thread, allowing queued
// HPX work to continue running.
//
// Senders executing through HPX should expose this domain through
// get_completion_domain<set_value_t> so sync_wait routes here.

namespace hpx::synchronization::detail::hpx_sync_wait_domain_impl {

    // Marker for the set_stopped completion path.
    template <typename T>
    struct stopped_t
    {
    };

    // Receiver env: exposes a stdexec run_loop scheduler via the standard
    // get_scheduler / get_delegation_scheduler queries so dependent
    // senders (let_value, let_error, etc.) can compute their completion
    // signatures against this environment. The run_loop is owned by
    // shared_state and is NEVER actually run; it exists purely as a
    // type carrier required by stdexec::sync_wait_t's constraint
    // (sender_in<sync_wait::__env>).
    //
    // Modern P2300: Uses query() functions instead of old tag_invoke.
    //
    // IMPORTANT:
    // We intentionally DO NOT provide a stop token.
    //
    // If we return never_stop_token, stdexec assumes the operation
    // can never be stopped, so it removes set_stopped from the
    // completion signatures.
    //
    // But our code may still call set_stopped().
    // That creates type mismatches and breaks things.
    //
    // By not exposing a stop token, stdexec keeps set_stopped
    // in the completion signatures, which matches our behavior.
    struct env
    {
        hpx::execution::experimental::run_loop* loop_ = nullptr;

        [[nodiscard]]
        constexpr auto query(
            hpx::execution::experimental::get_scheduler_t) const noexcept
        {
            return loop_->get_scheduler();
        }

        [[nodiscard]]
        constexpr auto query(
            hpx::execution::experimental::get_delegation_scheduler_t)
            const noexcept
        {
            return loop_->get_scheduler();
        }
    };

    template <typename... Ts>
    using decayed_tuple = std::tuple<std::decay_t<Ts>...>;

    template <typename Variant>
    struct first_alternative;

    template <typename T>
    struct first_alternative<std::variant<T>>
    {
        using type = T;
    };

    template <typename Sender>
    using value_tuple_for_t = typename first_alternative<
        hpx::execution::experimental::value_types_of_t<Sender, env,
            decayed_tuple, std::variant>>::type;

    template <typename ValueTuple>
    struct shared_state
    {
        hpx::spinlock mtx;
        hpx::condition_variable_any cv;
        hpx::execution::experimental::run_loop loop;
        bool done = false;
        std::variant<std::monostate, ValueTuple, std::exception_ptr,
            stopped_t<ValueTuple>>
            result;

        template <typename... Vs>
        void notify_value(Vs&&... vs) noexcept
        {
            try
            {
                {
                    std::unique_lock<hpx::spinlock> l(mtx);
                    result.template emplace<ValueTuple>(HPX_FORWARD(Vs, vs)...);
                    done = true;
                }
                cv.notify_all();
            }
            catch (...)
            {
                {
                    std::unique_lock<hpx::spinlock> l(mtx);
                    result.template emplace<std::exception_ptr>(
                        std::current_exception());
                    done = true;
                }
                cv.notify_all();
            }
        }

        template <typename E>
        void notify_error(E&& e) noexcept
        {
            {
                std::unique_lock<hpx::spinlock> l(mtx);
                using err_t = std::decay_t<E>;
                if constexpr (std::is_same_v<err_t, std::exception_ptr>)
                {
                    result.template emplace<std::exception_ptr>(
                        HPX_FORWARD(E, e));
                }
                else if constexpr (std::is_same_v<err_t, std::error_code>)
                {
                    result.template emplace<std::exception_ptr>(
                        std::make_exception_ptr(std::system_error(e)));
                }
                else
                {
                    try
                    {
                        throw HPX_FORWARD(E, e);
                    }
                    catch (...)
                    {
                        result.template emplace<std::exception_ptr>(
                            std::current_exception());
                    }
                }
                done = true;
            }
            cv.notify_all();
        }

        void notify_stopped() noexcept
        {
            {
                std::unique_lock<hpx::spinlock> l(mtx);
                result.template emplace<stopped_t<ValueTuple>>();
                done = true;
            }
            cv.notify_all();
        }

        std::optional<ValueTuple> wait_get_value()
        {
            {
                std::unique_lock<hpx::spinlock> l(mtx);
                cv.wait(l, [this] { return done; });
            }

            if (auto* v = std::get_if<ValueTuple>(&result))
            {
                return std::optional<ValueTuple>(HPX_MOVE(*v));
            }
            if (auto* ep = std::get_if<std::exception_ptr>(&result))
            {
                auto e = HPX_MOVE(*ep);
                std::rethrow_exception(HPX_MOVE(e));
            }
            // set_stopped
            return std::optional<ValueTuple>{};
        }
    };

    template <typename ValueTuple>
    struct receiver
    {
        using receiver_concept = hpx::execution::experimental::receiver_t;

        shared_state<ValueTuple>* state;

        template <typename... Vs>
        constexpr void set_value(Vs&&... vs) && noexcept
        {
            state->notify_value(HPX_FORWARD(Vs, vs)...);
        }

        template <typename E>
        constexpr void set_error(E&& e) && noexcept
        {
            state->notify_error(HPX_FORWARD(E, e));
        }

        constexpr void set_stopped() && noexcept
        {
            state->notify_stopped();
        }

        [[nodiscard]]
        constexpr env get_env() const noexcept
        {
            return env{&state->loop};
        }
    };

}    // namespace hpx::synchronization::detail::hpx_sync_wait_domain_impl

namespace hpx::synchronization::detail {

    // stdexec domain customizing only `apply_sender(sync_wait_t, ...)`
    // to use HPX-aware cooperative waiting.
    struct hpx_sync_wait_domain : hpx::execution::experimental::default_domain
    {
        template <hpx::execution::experimental::sender Sender>
        auto apply_sender(
            hpx::execution::experimental::sync_wait_t, Sender&& sndr) const
            -> std::optional<
                hpx_sync_wait_domain_impl::value_tuple_for_t<Sender>>
        {
            using value_tuple_t =
                hpx_sync_wait_domain_impl::value_tuple_for_t<Sender>;

            hpx_sync_wait_domain_impl::shared_state<value_tuple_t> state;

            auto op_state =
                hpx::execution::experimental::connect(HPX_FORWARD(Sender, sndr),
                    hpx_sync_wait_domain_impl::receiver<value_tuple_t>{&state});
            hpx::execution::experimental::start(op_state);
            return state.wait_get_value();
        }
    };

}    // namespace hpx::synchronization::detail
