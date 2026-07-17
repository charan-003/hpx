//  Copyright (c) 2026 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/// \file distributed_continues_on_sender.hpp
/// \brief P2300 sender adaptor for cross-locality value transfer.
///
/// This file is a placeholder for the distributed continues_on sender
/// adaptor. The hacky action-based implementation has been removed.
/// A proper component-based receiver marshalling system (nvexec-style)
/// will be implemented here in a future phase.

#include <hpx/config.hpp>

#if defined(HPX_HAVE_NETWORKING)

// TODO: Implement component-based receiver marshalling for distributed
// continues_on. The previous implementation dispatched dummy remote actions
// and attached .then() continuations to force execution hops. This approach
// was fundamentally broken for multi-value returns and exception propagation.
//
// The new implementation should:
//   1. Create an HPX component on the target locality that holds the
//      downstream receiver.
//   2. Marshal completion signals (set_value, set_error, set_stopped)
//      back across the network boundary via the component.
//   3. Support arbitrary value types through serialization.
//
// See: stdexec/nvexec for GPU/CPU boundary crossing patterns.

#endif    // HPX_HAVE_NETWORKING
