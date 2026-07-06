//  Copyright (c) 2025 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file transfer_action.hpp
/// \brief Declares the remote action endpoint for distributed P2300 scheduling.
///
/// This header declares a plain HPX action that can be sent over the
/// parcelport to execute work on a remote locality.  Phase 1 transmits
/// only a void signal (no value payload); serialized value forwarding
/// will be added in a later phase.

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

#include <hpx/modules/actions_base.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/naming_base.hpp>

namespace hpx::distributed::detail {

    /// Phase 1: void->void function for basic schedule()
    HPX_EXPORT void distributed_execute_void();

    HPX_EXPORT hpx::future<void> dispatch_distributed_execute_void(
        hpx::id_type const& target);

    /// Phase 2: template function that takes an hpx::tuple of values
    /// and returns the same tuple, effectively passing it across the network.
    template <typename... Ts>
    HPX_EXPORT hpx::tuple<Ts...> distributed_execute_value(
        hpx::tuple<Ts...> const& t);

    /// A helper function to asynchronously dispatch the action and return
    /// the future, keeping the action type instantiation in the source file.
    template <typename... Ts>
    HPX_EXPORT hpx::future<hpx::tuple<Ts...>>
    dispatch_distributed_execute_value(
        hpx::id_type const& target, hpx::tuple<Ts...> const& t);

}    // namespace hpx::distributed::detail

// Forward-declare the template action explicitly to avoid macro parsing issues.
struct distributed_execute_value_action_int
  : hpx::actions::make_action_t<
        decltype(&hpx::distributed::detail::distributed_execute_value<int>),
        &hpx::distributed::detail::distributed_execute_value<int>,
        distributed_execute_value_action_int>
{};

HPX_DECLARE_PLAIN_ACTION(hpx::distributed::detail::distributed_execute_void,
    distributed_execute_void_action)

#endif    // HPX_HAVE_NETWORKING
