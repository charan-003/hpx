//  Copyright (c) 2026 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/// \file transfer_action.hpp
/// \brief Declares the remote action endpoint for distributed P2300 scheduling.
///
/// This header declares a plain HPX action that can be sent over the
/// parcelport to execute work on a remote locality.  Phase 1 transmits
/// only a void signal (no value payload); serialized value forwarding
/// will be added in a later phase.

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

#include <hpx/modules/actions_base.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/naming_base.hpp>

#include <functional>
#include <type_traits>

namespace hpx::distributed::detail {

    template <typename F, typename... Ts>
    inline std::invoke_result_t<F, Ts...> distributed_invoke_callable(
        F f, Ts... vals)
    {
        return HPX_INVOKE(f, HPX_MOVE(vals)...);
    }

    /// Generic struct representing an HPX action that invokes a user-defined callable
    /// across the network. The callable F and arguments Ts... must be serializable.
    template <typename F, typename... Ts>
    struct distributed_invoke_callable_action
      : hpx::actions::make_action_t<
            decltype(&hpx::distributed::detail::distributed_invoke_callable<F,
                Ts...>),
            &hpx::distributed::detail::distributed_invoke_callable<F, Ts...>,
            distributed_invoke_callable_action<F, Ts...>>
    {
    };

}    // namespace hpx::distributed::detail

#endif    // HPX_HAVE_NETWORKING
