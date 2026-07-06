//  Copyright (c) 2025 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file distributed_just.hpp
/// \brief P2300-compliant sender for cross-locality value forwarding.
///
/// Provides a sender `distributed_just` that packages arbitrary values
/// into a tuple, transmits them to a remote HPX locality via the parcelport,
/// and unpacks them into the downstream receiver's set_value().

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

#include <hpx/async_distributed/detail/transfer_action.hpp>
#include <hpx/datastructures/tuple.hpp>
#include <hpx/functional/invoke_fused.hpp>
#include <hpx/include/async.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/execution_base.hpp>
#include <hpx/modules/futures.hpp>

#include <exception>
#include <tuple>
#include <type_traits>
#include <utility>

namespace hpx::distributed {

    template <typename... Ts>
    struct distributed_just_sender;

    template <typename Receiver, typename... Ts>
    struct distributed_just_operation_state;

    ///////////////////////////////////////////////////////////////////////////
    template <typename Receiver, typename... Ts>
    struct distributed_just_operation_state
    {
        using receiver_type = std::decay_t<Receiver>;

        template <typename Receiver_>
        distributed_just_operation_state(
            Receiver_&& r, hpx::id_type target, hpx::tuple<Ts...> vals) noexcept
          : receiver_(HPX_FORWARD(Receiver_, r))
          , target_(HPX_MOVE(target))
          , vals_(HPX_MOVE(vals))
        {
        }

        distributed_just_operation_state(
            distributed_just_operation_state const&) = delete;
        distributed_just_operation_state& operator=(
            distributed_just_operation_state const&) = delete;
        distributed_just_operation_state(
            distributed_just_operation_state&&) = delete;
        distributed_just_operation_state& operator=(
            distributed_just_operation_state&&) = delete;

        ~distributed_just_operation_state() = default;

        void start() & noexcept
        {
            hpx::detail::try_catch_exception_ptr(
                [&]() {
                    // Dispatch the tuple to the remote locality.
                    auto fut = hpx::distributed::detail::
                        dispatch_distributed_execute_value<Ts...>(
                            target_, vals_);

                    // Chain a continuation to bridge the future completion
                    fut.then([this](hpx::future<hpx::tuple<Ts...>> f) {
                        hpx::detail::try_catch_exception_ptr(
                            [&]() {
                                // Extract the returned tuple from the future
                                auto returned_tuple = f.get();

                                // Unpack the tuple and pass to set_value
                                hpx::invoke_fused(
                                    [&](auto&&... args) {
                                        hpx::execution::experimental::set_value(
                                            HPX_MOVE(receiver_),
                                            std::forward<decltype(args)>(
                                                args)...);
                                    },
                                    HPX_MOVE(returned_tuple));
                            },
                            [&](std::exception_ptr ep) {
                                hpx::execution::experimental::set_error(
                                    HPX_MOVE(receiver_), HPX_MOVE(ep));
                            });
                    });
                },
                [&](std::exception_ptr ep) {
                    hpx::execution::experimental::set_error(
                        HPX_MOVE(receiver_), HPX_MOVE(ep));
                });
        }

    private:
        receiver_type receiver_;
        hpx::id_type target_;
        hpx::tuple<Ts...> vals_;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename... Ts>
    struct distributed_just_sender
    {
        using sender_concept = hpx::execution::experimental::sender_t;

        using completion_signatures =
            hpx::execution::experimental::completion_signatures<
                hpx::execution::experimental::set_value_t(Ts...),
                hpx::execution::experimental::set_error_t(std::exception_ptr),
                hpx::execution::experimental::set_stopped_t()>;

        explicit distributed_just_sender(
            hpx::id_type target, Ts... vals) noexcept
          : target_(HPX_MOVE(target))
          , vals_(hpx::make_tuple(HPX_MOVE(vals)...))
        {
        }

        distributed_just_sender(distributed_just_sender&&) = default;
        distributed_just_sender& operator=(distributed_just_sender&&) = default;
        distributed_just_sender(distributed_just_sender const&) = default;
        distributed_just_sender& operator=(
            distributed_just_sender const&) = default;

        ~distributed_just_sender() = default;

        template <typename Receiver>
        distributed_just_operation_state<Receiver, Ts...> connect(
            Receiver&& receiver) &&
        {
            return {HPX_FORWARD(Receiver, receiver), HPX_MOVE(target_),
                HPX_MOVE(vals_)};
        }

        template <typename Receiver>
        distributed_just_operation_state<Receiver, Ts...> connect(
            Receiver&& receiver) const&
        {
            return {HPX_FORWARD(Receiver, receiver), target_, vals_};
        }

        auto get_env() const noexcept
        {
            return hpx::execution::experimental::empty_env{};
        }

    private:
        hpx::id_type target_;
        hpx::tuple<Ts...> vals_;
    };

    ///////////////////////////////////////////////////////////////////////////
    /// CPO distributed_just(target, args...)
    inline constexpr struct distributed_just_t final
    {
        template <typename... Ts>
        constexpr auto operator()(hpx::id_type target, Ts&&... ts) const
        {
            return distributed_just_sender<std::decay_t<Ts>...>{
                HPX_MOVE(target), HPX_FORWARD(Ts, ts)...};
        }
    } distributed_just{};

}    // namespace hpx::distributed

#endif    // HPX_HAVE_NETWORKING
