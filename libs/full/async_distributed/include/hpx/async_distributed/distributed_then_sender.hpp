//  Copyright (c) 2026 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

/// \file distributed_then_sender.hpp
/// \brief P2300 sender adaptor for remote then() execution.
///
/// This file is a placeholder for the distributed then sender adaptor.
/// The hacky action-based implementation (distributed_invoke_callable_action)
/// has been removed. A proper component-based receiver marshalling system
/// (nvexec-style) will be implemented here in a future phase.

// TODO: Implement component-based receiver marshalling for distributed
// then(). The previous implementation used distributed_invoke_callable_action
// to execute callables on remote localities, but this approach:
//   - Could not handle non-serializable callables (lambdas).
//   - Required explicit action registration per callable type.
//   - Struggled with multi-value returns.
//   - Had unsafe destructor semantics with continuation futures.
//
// The new implementation should:
//   1. Create an HPX component on the target locality.
//   2. Marshal the callable and its arguments to the component.
//   3. Execute the callable remotely and marshal the result back.
//   4. Support arbitrary return types through serialization.
//
// See: stdexec/nvexec for GPU/CPU boundary crossing patterns.

#endif    // HPX_HAVE_NETWORKING
