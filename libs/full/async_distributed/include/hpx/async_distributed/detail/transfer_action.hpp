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

    /// Phase 2: template function that takes an hpx::tuple of values
    /// and returns the same tuple, effectively passing it across the network.
    template <typename... Ts>
    inline hpx::tuple<Ts...> distributed_execute_value(
        hpx::tuple<Ts...> const& t)
    {
        return t;
    }

}    // namespace hpx::distributed::detail

// Expose a specific struct for the scheduler's empty payload signal
struct distributed_schedule_action
  : hpx::actions::make_action_t<
        decltype(&hpx::distributed::detail::distributed_execute_value<>),
        &hpx::distributed::detail::distributed_execute_value<>,
        distributed_schedule_action>
{
};

namespace hpx::distributed::detail {

    /// A helper function to asynchronously dispatch the action and return
    /// the future. For arbitrary types, this must be specialized or
    /// resolved by the user (or tests) who registers the specific action.
    template <typename... Ts>
    hpx::future<hpx::tuple<Ts...>> dispatch_distributed_execute_value(
        hpx::id_type const& target, hpx::tuple<Ts...> const& t);

    /// Overload for the empty payload (used natively by schedule())
    inline hpx::future<hpx::tuple<>> dispatch_distributed_execute_value(
        hpx::id_type const& target, hpx::tuple<> const& t)
    {
        return hpx::async(distributed_schedule_action{}, target, t);
    }

}    // namespace hpx::distributed::detail

#endif    // HPX_HAVE_NETWORKING
