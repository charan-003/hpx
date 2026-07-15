//  Copyright (c) 2026 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

#include <hpx/async_distributed/detail/transfer_action.hpp>
#include <hpx/execution_base/completion_signatures.hpp>
#include <hpx/execution_base/receiver.hpp>
#include <hpx/execution_base/sender.hpp>
#include <hpx/modules/errors.hpp>

#include <exception>
#include <type_traits>
#include <utility>

namespace hpx::distributed::experimental::detail {

    template <typename Receiver, typename F>
    struct distributed_then_receiver
    {
        using receiver_type = std::decay_t<Receiver>;
        using receiver_concept = hpx::execution::experimental::receiver_t;

        HPX_NO_UNIQUE_ADDRESS receiver_type downstream_;
        F f_;
        hpx::id_type target_;
        hpx::future<void>& continuation_future_;

        template <typename... Ts>
        void set_value(Ts... vals) && noexcept
        {
            hpx::detail::try_catch_exception_ptr(
                [&]() {
                    // Dispatch the remote invocation action
                    auto fut = hpx::async(
                        hpx::distributed::detail::
                            distributed_invoke_callable_action<F, Ts...>{},
                        target_, HPX_MOVE(f_), HPX_MOVE(vals)...);

                    // Chain continuation to forward the remote result
                    continuation_future_ = fut.then(
                        [downstream = HPX_MOVE(downstream_)](auto&& f) mutable {
                            hpx::detail::try_catch_exception_ptr(
                                [&]() {
                                    using result_type =
                                        std::invoke_result_t<F, Ts...>;
                                    if constexpr (std::is_void_v<result_type>)
                                    {
                                        f.get();    // wait for completion
                                        hpx::execution::experimental::set_value(
                                            HPX_MOVE(downstream));
                                    }
                                    else
                                    {
                                        auto result = f.get();
                                        hpx::execution::experimental::set_value(
                                            HPX_MOVE(downstream),
                                            HPX_MOVE(result));
                                    }
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

    template <typename Sender, typename F>
    struct distributed_then_sender
    {
        using sender_concept = hpx::execution::experimental::sender_t;
        using upstream_env_t = hpx::execution::experimental::env_of_t<Sender>;

        HPX_NO_UNIQUE_ADDRESS Sender sender_;
        F f_;
        hpx::id_type target_;

        template <typename Env>
        struct generate_completion_signatures
        {
            struct domainless_sender
            {
                using sender_concept = hpx::execution::experimental::sender_t;
                using completion_signatures =
                    hpx::execution::experimental::completion_signatures_of_t<
                        Sender, Env>;

                auto get_env() const noexcept
                {
                    return hpx::execution::experimental::empty_env{};
                }
            };

            using type =
                hpx::execution::experimental::completion_signatures_of_t<
                    decltype(hpx::execution::experimental::then(
                        std::declval<domainless_sender>(), std::declval<F>())),
                    Env>;
        };

        template <typename Env>
        friend auto tag_invoke(
            hpx::execution::experimental::get_completion_signatures_t,
            distributed_then_sender const&, Env&&) noexcept ->
            typename generate_completion_signatures<Env>::type;

        template <typename Receiver>
        struct operation_state
        {
            using upstream_op_t =
                decltype(hpx::execution::experimental::connect(
                    std::declval<Sender>(),
                    std::declval<distributed_then_receiver<Receiver, F>>()));

            upstream_op_t upstream_op_;
            hpx::future<void> continuation_future_;

            template <typename Sender_, typename Receiver_>
            operation_state(
                Sender_&& sndr, Receiver_&& rcvr, F f, hpx::id_type target)
              : upstream_op_(hpx::execution::experimental::connect(
                    HPX_FORWARD(Sender_, sndr),
                    distributed_then_receiver<Receiver, F>{
                        HPX_FORWARD(Receiver_, rcvr), HPX_MOVE(f),
                        HPX_MOVE(target), continuation_future_}))
            {
            }

            ~operation_state()
            {
                if (continuation_future_.valid() &&
                    continuation_future_.has_exception())
                {
                    continuation_future_.wait();
                }
            }

            friend void tag_invoke(hpx::execution::experimental::start_t,
                operation_state& os) noexcept
            {
                hpx::execution::experimental::start(os.upstream_op_);
            }
        };

        template <typename Receiver>
        friend operation_state<Receiver> tag_invoke(
            hpx::execution::experimental::connect_t,
            distributed_then_sender&& s, Receiver&& receiver)
        {
            return operation_state<Receiver>{HPX_MOVE(s.sender_),
                HPX_FORWARD(Receiver, receiver), HPX_MOVE(s.f_),
                HPX_MOVE(s.target_)};
        }

        template <typename Receiver>
        friend operation_state<Receiver> tag_invoke(
            hpx::execution::experimental::connect_t,
            distributed_then_sender const& s, Receiver&& receiver)
        {
            return operation_state<Receiver>{
                s.sender_, HPX_FORWARD(Receiver, receiver), s.f_, s.target_};
        }

        auto get_env() const noexcept
        {
            return hpx::execution::experimental::get_env(sender_);
        }
    };

}    // namespace hpx::distributed::experimental::detail

#endif    // HPX_HAVE_NETWORKING
