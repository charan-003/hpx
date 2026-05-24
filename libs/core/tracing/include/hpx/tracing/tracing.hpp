//  Copyright (c) 2026 Hartmut Kaiser
//  Copyright (c) 2026 Vansh Dobhal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_TRACY)
#include <hpx/tracing/backends/tracy.hpp>
#elif defined(HPX_HAVE_ITTNOTIFY) && HPX_HAVE_ITTNOTIFY != 0
#include <hpx/tracing/backends/ittnotify.hpp>
#elif defined(HPX_HAVE_APEX)
#include <hpx/tracing/backends/apex.hpp>
#else
#include <hpx/tracing/backends/empty.hpp>
#endif
