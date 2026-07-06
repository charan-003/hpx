//  Copyright (c) 2025 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file distributed_scheduler.hpp
/// \brief P2300-compliant scheduler for cross-locality execution.
///
/// Provides a scheduler that dispatches work to a remote HPX locality
/// via the parcelport, bridging the hpx::future completion back into
/// the P2300 set_value / set_error receiver protocol.
///
/// Usage:
/// \code
///   hpx::id_type remote = hpx::find_all_localities()[1];
///   auto sched = hpx::distributed::distributed_scheduler{remote};
///   auto result = sched.schedule()
///       | stdexec::then([]{ return 42; })
///       | stdexec::sync_wait();
/// \endcode

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

#include <hpx/async_distributed/detail/async_implementations.hpp>
#include <hpx/async_distributed/detail/transfer_action.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/execution_base.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/naming_base.hpp>

#include <exception>
#include <type_traits>
#include <utility>

namespace hpx::distributed {

    // Forward declarations
    struct distributed_scheduler;
    struct distributed_schedule_sender;

    template <typename Receiver>
    struct distributed_operation_state;

    ///////////////////////////////////////////////////////////////////////////
    // Operation state: bridges the hpx::future<void> returned by the remote
    // action into the P2300 receiver protocol.
    //
    // Lifetime contract (P2300 section 6.9.7):
    //   The operation_state is pinned in memory by the caller and must
    //   outlive the async operation.  We capture `this` in the .then()
    //   continuation, which is safe because the future's shared state
    //   prevents destruction until the continuation fires.
    template <typename Receiver>
    struct distributed_operation_state
    {
        using receiver_type = std::decay_t<Receiver>;

        template <typename Receiver_>
        distributed_operation_state(Receiver_&& r, hpx::id_type target) noexcept
          : receiver_(HPX_FORWARD(Receiver_, r))
          , target_(HPX_MOVE(target))
        {
        }

        distributed_operation_state(
            distributed_operation_state const&) = delete;
        distributed_operation_state& operator=(
            distributed_operation_state const&) = delete;
        distributed_operation_state(distributed_operation_state&&) = delete;
        distributed_operation_state& operator=(
            distributed_operation_state&&) = delete;

        ~distributed_operation_state() = default;

        void start() & noexcept
        {
            hpx::detail::try_catch_exception_ptr(
                [&]() {
                    // Dispatch the void action to the remote locality.
                    auto fut = hpx::distributed::detail::
                        dispatch_distributed_execute_void(target_);

                    // Chain a continuation to bridge the future completion
                    // into the P2300 receiver.  We capture `this` because
                    // the operation_state is pinned (P2300 lifetime
                    // guarantee).
                    fut.then([this](hpx::future<void> f) {
                        hpx::detail::try_catch_exception_ptr(
                            [&]() {
                                f.get();    // rethrow any remote exception
                                hpx::execution::experimental::set_value(
                                    HPX_MOVE(receiver_));
                            },
                            [&](std::exception_ptr ep) {
                                hpx::execution::experimental::set_error(
                                    HPX_MOVE(receiver_), HPX_MOVE(ep));
                            });
                    });
                },
                [&](std::exception_ptr ep) {
                    // If hpx::async itself throws (e.g. invalid target),
                    // route the error to the receiver.
                    hpx::execution::experimental::set_error(
                        HPX_MOVE(receiver_), HPX_MOVE(ep));
                });
        }

    private:
        receiver_type receiver_;
        hpx::id_type target_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Sender returned by distributed_scheduler::schedule().
    //
    // Completion signatures:
    //   set_value()             - remote action completed successfully
    //   set_error(exception_ptr) - remote action or network transport failed
    //   set_stopped()           - included for concept conformance
    struct distributed_schedule_sender
    {
        using sender_concept = hpx::execution::experimental::sender_t;

        using completion_signatures =
            hpx::execution::experimental::completion_signatures<
                hpx::execution::experimental::set_value_t(),
                hpx::execution::experimental::set_error_t(std::exception_ptr),
                hpx::execution::experimental::set_stopped_t()>;

        explicit distributed_schedule_sender(hpx::id_type target) noexcept
          : target_(HPX_MOVE(target))
        {
        }

        distributed_schedule_sender(distributed_schedule_sender&&) = default;
        distributed_schedule_sender& operator=(
            distributed_schedule_sender&&) = default;
        distributed_schedule_sender(
            distributed_schedule_sender const&) = default;
        distributed_schedule_sender& operator=(
            distributed_schedule_sender const&) = default;

        ~distributed_schedule_sender() = default;

        template <typename Receiver>
        distributed_operation_state<Receiver> connect(Receiver&& receiver) &&
        {
            return {HPX_FORWARD(Receiver, receiver), HPX_MOVE(target_)};
        }

        template <typename Receiver>
        distributed_operation_state<Receiver> connect(
            Receiver&& receiver) const&
        {
            return {HPX_FORWARD(Receiver, receiver), target_};
        }

        struct env
        {
            distributed_scheduler const& sched;

            template <typename CPO>
                requires meta::value<
                    meta::one_of<CPO, hpx::execution::experimental::set_value_t,
                        hpx::execution::experimental::set_stopped_t>>
            auto query(
                hpx::execution::experimental::get_completion_scheduler_t<CPO>)
                const noexcept;
        };

        auto get_env() const noexcept;

    private:
        // Allow env to access the scheduler stored in the sender.
        // The scheduler is reconstructed from target_ on demand.
        hpx::id_type target_;

        // The sender needs to remember which scheduler created it so
        // that get_completion_scheduler can return it.
        friend struct distributed_scheduler;
        friend struct env;
    };

    ///////////////////////////////////////////////////////////////////////////
    // P2300-compliant scheduler that dispatches work to a remote HPX
    // locality via the parcelport.
    //
    // Satisfies the stdexec::scheduler concept:
    //   - schedule() returns a sender
    //   - equality-comparable
    //   - copy-constructible
    struct distributed_scheduler
    {
        explicit distributed_scheduler(hpx::id_type target) noexcept
          : target_(HPX_MOVE(target))
        {
        }

        distributed_scheduler(distributed_scheduler const&) = default;
        distributed_scheduler& operator=(
            distributed_scheduler const&) = default;
        distributed_scheduler(distributed_scheduler&&) = default;
        distributed_scheduler& operator=(distributed_scheduler&&) = default;

        ~distributed_scheduler() = default;

        /// P2300 schedule CPO: returns a sender that, when started,
        /// dispatches a void action to the target locality.
        [[nodiscard]] distributed_schedule_sender schedule() const
        {
            return distributed_schedule_sender{target_};
        }

        friend bool operator==(distributed_scheduler const& lhs,
            distributed_scheduler const& rhs) noexcept
        {
            return lhs.target_ == rhs.target_;
        }

        friend bool operator!=(distributed_scheduler const& lhs,
            distributed_scheduler const& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        /// Returns the target locality this scheduler dispatches to.
        [[nodiscard]] hpx::id_type const& target() const noexcept
        {
            return target_;
        }

    private:
        hpx::id_type target_;
    };

    // ---- Deferred inline definitions (after distributed_scheduler is
    //      complete) ----

    template <typename CPO>
        requires meta::value<
            meta::one_of<CPO, hpx::execution::experimental::set_value_t,
                hpx::execution::experimental::set_stopped_t>>
    auto distributed_schedule_sender::env::query(
        hpx::execution::experimental::get_completion_scheduler_t<CPO>)
        const noexcept
    {
        return distributed_scheduler{sched.target()};
    }

    inline auto distributed_schedule_sender::get_env() const noexcept
    {
        // Reconstruct the scheduler from the stored target id.
        // This is cheap: distributed_scheduler holds only an id_type.
        return env{distributed_scheduler{target_}};
    }

}    // namespace hpx::distributed

#endif    // HPX_HAVE_NETWORKING
