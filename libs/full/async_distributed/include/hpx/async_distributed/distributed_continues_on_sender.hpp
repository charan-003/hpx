#pragma once

//  Copyright (c) 2025 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file distributed_transfer_sender.hpp
/// \brief P2300 sender adaptor for cross-locality value transfer.
///
/// Implements the `distributed_continues_on_sender<Sender>` adaptor which
/// connects to an upstream sender, intercepts its set_value(args...),
/// serializes the values into an hpx::tuple, dispatches them to a remote
/// locality via the parcelport, and unpacks the result into a downstream
/// receiver.
///
/// This enables the standard pipeline:
///   distributed_continues_on(ex::just(42), distributed_scheduler(target))
///     | ex::then([](int x) { return x * 2; })

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

#include <hpx/async_distributed/detail/transfer_action.hpp>
#include <hpx/datastructures/tuple.hpp>
#include <hpx/functional/invoke_fused.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/execution_base.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/naming_base.hpp>

#include <exception>
#include <type_traits>
#include <utility>

namespace hpx::distributed::experimental {

    // Forward declarations
    template <typename Sender>
    struct distributed_continues_on_sender;

    ///////////////////////////////////////////////////////////////////////////
    /// The transfer_receiver sits between the upstream sender and the
    /// downstream receiver.  When the upstream fires set_value(args...),
    /// we serialize the values and dispatch them to the remote locality.
    /// It stores the .then() continuation future in a slot provided by
    /// the operation_state to prevent use-after-free at shutdown.
    template <typename Receiver, typename... Ts>
    struct distributed_continues_on_receiver
    {
        using receiver_type = std::decay_t<Receiver>;
        using receiver_concept = hpx::execution::experimental::receiver_t;

        HPX_NO_UNIQUE_ADDRESS receiver_type downstream_;
        hpx::id_type target_;

        void set_value(Ts... vals) && noexcept
        {
            hpx::detail::try_catch_exception_ptr(
                [&]() {
                    auto t = hpx::make_tuple(HPX_MOVE(vals)...);

                    auto fut = hpx::distributed::detail::
                        dispatch_distributed_execute_value<Ts...>(target_, t);

                    // Dispatch continuation directly without storing the future
                    fut.then([downstream = HPX_MOVE(downstream_)](
                                 hpx::future<hpx::tuple<Ts...>>&& f) mutable {
                        hpx::detail::try_catch_exception_ptr(
                            [&]() {
                                auto returned_tuple = f.get();
                                hpx::invoke_fused(
                                    [&](auto&&... args) {
                                        hpx::execution::experimental::set_value(
                                            HPX_MOVE(downstream),
                                            HPX_FORWARD(
                                                decltype(args), args)...);
                                    },
                                    HPX_MOVE(returned_tuple));
                            },
                            [&](std::exception_ptr ep) {
                                hpx::execution::experimental::set_error(
                                    HPX_MOVE(downstream), HPX_MOVE(ep));
                            });
                    });
                },
                [&](std::exception_ptr ep) {
                    hpx::execution::experimental::set_error(
                        HPX_MOVE(downstream_), HPX_MOVE(ep));
                });
        }

        void set_error(std::exception_ptr ep) && noexcept
        {
            hpx::execution::experimental::set_error(
                HPX_MOVE(downstream_), HPX_MOVE(ep));
        }

        void set_stopped() && noexcept
        {
            hpx::execution::experimental::set_stopped(HPX_MOVE(downstream_));
        }

        auto get_env() const noexcept
        {
            return hpx::execution::experimental::get_env(downstream_);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Operation state for the transfer adaptor.
    /// Connects the upstream sender to our transfer_receiver, which
    /// intercepts set_value and bounces it through the network.
    /// Holds the continuation future to keep it alive until completion.
    template <typename Sender, typename Receiver, typename... Ts>
    struct distributed_continues_on_operation_state
    {
        using upstream_receiver_t =
            distributed_continues_on_receiver<Receiver, Ts...>;
        using upstream_op_t = decltype(hpx::execution::experimental::connect(
            std::declval<Sender>(), std::declval<upstream_receiver_t>()));

        upstream_op_t upstream_op_;

        template <typename Sender_, typename Receiver_>
        distributed_continues_on_operation_state(
            Sender_&& sndr, Receiver_&& rcvr, hpx::id_type target)
          : upstream_op_(hpx::execution::experimental::connect(
                HPX_FORWARD(Sender_, sndr),
                upstream_receiver_t{
                    HPX_FORWARD(Receiver_, rcvr), HPX_MOVE(target)}))
        {
        }

        distributed_continues_on_operation_state(
            distributed_continues_on_operation_state const&) = delete;
        distributed_continues_on_operation_state& operator=(
            distributed_continues_on_operation_state const&) = delete;
        distributed_continues_on_operation_state(
            distributed_continues_on_operation_state&&) = delete;
        distributed_continues_on_operation_state& operator=(
            distributed_continues_on_operation_state&&) = delete;

        ~distributed_continues_on_operation_state() = default;

        void start() & noexcept
        {
            hpx::execution::experimental::start(upstream_op_);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    /// Sender adaptor for distributed transfer.
    ///
    /// Wraps an upstream sender.  When connected, it creates a
    /// transfer_receiver that intercepts set_value from the upstream,
    /// serializes the values, sends them to the target locality, and
    /// delivers them to the downstream receiver.
    ///
    /// For Phase 3, this only supports upstream senders with a single
    /// set_value_t(int) completion signature (since our action is only
    /// registered for int).  This restriction will be lifted when we
    /// register additional type inst instantiations.
    template <typename Sender>
    struct distributed_continues_on_sender
    {
        using sender_concept = hpx::execution::experimental::sender_t;

        // Phase 3: the upstream sends int, so after transfer we still send int.
        // The completion signatures match the upstream but add exception_ptr.
        using completion_signatures =
            hpx::execution::experimental::completion_signatures<
                hpx::execution::experimental::set_value_t(int),
                hpx::execution::experimental::set_error_t(std::exception_ptr),
                hpx::execution::experimental::set_stopped_t()>;

        HPX_NO_UNIQUE_ADDRESS Sender upstream_;
        hpx::id_type target_;

        template <typename Sender_>
        explicit distributed_continues_on_sender(
            Sender_&& sndr, hpx::id_type target)
          : upstream_(HPX_FORWARD(Sender_, sndr))
          , target_(HPX_MOVE(target))
        {
        }

        distributed_continues_on_sender(
            distributed_continues_on_sender&&) = default;
        distributed_continues_on_sender& operator=(
            distributed_continues_on_sender&&) = default;
        distributed_continues_on_sender(
            distributed_continues_on_sender const&) = default;
        distributed_continues_on_sender& operator=(
            distributed_continues_on_sender const&) = default;

        ~distributed_continues_on_sender() = default;

        template <typename Receiver>
        auto connect(Receiver&& receiver) &&
        {
            return distributed_continues_on_operation_state<Sender, Receiver,
                int>{HPX_MOVE(upstream_), HPX_FORWARD(Receiver, receiver),
                HPX_MOVE(target_)};
        }

        template <typename Receiver>
        auto connect(Receiver&& receiver) const&
        {
            return distributed_continues_on_operation_state<Sender, Receiver,
                int>{upstream_, HPX_FORWARD(Receiver, receiver), target_};
        }

        auto get_env() const noexcept
        {
            return hpx::execution::experimental::empty_env{};
        }
    };

    namespace detail {
        struct distributed_domain;
    }
}    // namespace hpx::distributed::experimental

namespace hpx::distributed::experimental::detail {
    // Implement transform_sender for continues_on_t
    template <typename Sender, typename Scheduler>
    constexpr auto tag_invoke(hpx::execution::experimental::transform_sender_t,
        distributed_domain, hpx::execution::experimental::continues_on_t,
        Sender&& sndr, Scheduler const& sched) noexcept
    {
        return distributed_continues_on_sender<std::decay_t<Sender>>{
            HPX_FORWARD(Sender, sndr), sched.target()};
    }
}    // namespace hpx::distributed::experimental::detail

#endif    // HPX_HAVE_NETWORKING
