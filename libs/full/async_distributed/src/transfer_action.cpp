//  Copyright (c) 2025 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)
#if defined(HPX_HAVE_NETWORKING)

// The full continuation definition must be visible before
// HPX_PLAIN_ACTION instantiates the transfer_action template.
#include <hpx/async_distributed/async.hpp>
#include <hpx/async_distributed/continuation.hpp>
#include <hpx/async_distributed/detail/transfer_action.hpp>
#include <hpx/modules/actions.hpp>

namespace hpx::distributed::detail {

    void distributed_execute_void() {}

    template <typename... Ts>
    hpx::tuple<Ts...> distributed_execute_value(hpx::tuple<Ts...> const& t)
    {
        // Phase 2: simply return the tuple. The purpose of this action
        // is to serialize the value tuple across the network and
        // reconstruct it on the other side.
        return t;
    }

}    // namespace hpx::distributed::detail

// Register the action with the HPX serialization framework.
HPX_PLAIN_ACTION(hpx::distributed::detail::distributed_execute_void,
    distributed_execute_void_action)

// We explicitly instantiate and register it for `int` for Phase 2 testing.
HPX_REGISTER_ACTION(
    distributed_execute_value_action_int,
    distributed_execute_value_action_int)

namespace hpx::distributed::detail {

    hpx::future<void> dispatch_distributed_execute_void(
        hpx::id_type const& target)
    {
        return hpx::async(distributed_execute_void_action{}, target);
    }

    template <>
    hpx::future<hpx::tuple<int>> dispatch_distributed_execute_value<int>(
        hpx::id_type const& target, hpx::tuple<int> const& t)
    {
        return hpx::async(distributed_execute_value_action_int{}, target, t);
    }

}    // namespace hpx::distributed::detail

#endif    // HPX_HAVE_NETWORKING
#endif    // HPX_COMPUTE_DEVICE_CODE
