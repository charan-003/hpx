// Copyright (c) 2026 The STE||AR Group
// Copyright (c) 2026 Vansh Dobhal
//
// SPDX-License-Identifier: BSL-1.0
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This smoke test verifies that HPX compiles, links, and runs correctly
// with ITTNotify support enabled. The scheduling loop automatically
// exercises ITT instrumentation (task regions, caller contexts) when
// HPX_WITH_ITTNOTIFY=ON.

#include <hpx/future.hpp>
#include <hpx/init.hpp>

#include <cstddef>

int hpx_main()
{
    // Spawn a few tasks so the scheduler exercises its ITT instrumentation
    constexpr std::size_t n = 8;
    hpx::future<int> futs[n];

    for (std::size_t i = 0; i < n; ++i)
    {
        futs[i] = hpx::async([i]() -> int { return static_cast<int>(i * i); });
    }

    for (auto& f : futs)
    {
        f.get();
    }

    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::local::init(hpx_main, argc, argv);
}
