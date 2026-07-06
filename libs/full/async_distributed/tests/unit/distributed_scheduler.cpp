//  Copyright (c) 2025 Shivansh Singh
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Test: distributed_scheduler local round-trip via P2300 sender protocol.
//
// This test verifies that:
//   1. distributed_scheduler satisfies the P2300 scheduler concept.
//   2. schedule() returns a valid sender.
//   3. The sender can be connected to a receiver and started.
//   4. The operation completes successfully when targeting the local
//      locality (hpx::find_here()).

#include <hpx/config.hpp>
#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/hpx_init.hpp>
#include <hpx/include/runtime.hpp>
#include <hpx/modules/execution.hpp>
#include <hpx/modules/testing.hpp>

#include <hpx/async_distributed/distributed_just.hpp>
#include <hpx/async_distributed/distributed_scheduler.hpp>
#include <hpx/async_distributed/distributed_transfer_sender.hpp>

#include <atomic>
#include <exception>

namespace ex = hpx::execution::experimental;
namespace tt = hpx::this_thread::experimental;

///////////////////////////////////////////////////////////////////////////////
int hpx_main()
{
    // Test 1: schedule() on the local locality completes with set_value.
    {
        auto sched = hpx::distributed::distributed_scheduler{hpx::find_here()};
        auto s = sched.schedule();

        // Verify the sender satisfies the sender concept.
        static_assert(ex::sender<decltype(s)>,
            "distributed_schedule_sender must satisfy the sender concept");

        // Use cooperative sync_wait to avoid deadlocking the HPX runtime.
        tt::sync_wait(HPX_MOVE(s));
    }

    // Test 2: schedule() piped through then() produces a value.
    {
        hpx::id_type here = hpx::find_here();
        auto s1 = ex::schedule(hpx::distributed::distributed_scheduler(here));

        auto s2 = ex::then(s1, []() { return 42; });
        auto result = tt::sync_wait(s2);

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(*result), 42);
    }

    // Test 3: scheduler equality.
    {
        auto here = hpx::find_here();
        auto s1 = hpx::distributed::distributed_scheduler{here};
        auto s2 = hpx::distributed::distributed_scheduler{here};
        HPX_TEST(s1 == s2);
    }

    // Test 4: distributed_just passing a value across the wire
    {
        auto result = tt::sync_wait(
            hpx::distributed::distributed_just(hpx::find_here(), 42));

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(*result), 42);
    }

    // Test 5: distributed_continues_on pipeline
    //   distributed_continues_on(just(10), sched)
    //     | then([](int x) { return x * 2; }) | sync_wait()
    {
        auto sched = hpx::distributed::distributed_scheduler{hpx::find_here()};

        auto transferred =
            hpx::distributed::distributed_continues_on(ex::just(10), sched);

        auto result = tt::sync_wait(
            HPX_MOVE(transferred) | ex::then([](int x) { return x * 2; }));

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(*result), 20);
    }

    // Test 6: chained distributed_continues_on with additional then()
    {
        auto sched = hpx::distributed::distributed_scheduler{hpx::find_here()};

        auto transferred =
            hpx::distributed::distributed_continues_on(ex::just(7), sched);

        auto result = tt::sync_wait(
            HPX_MOVE(transferred) | ex::then([](int x) { return x + 3; }));

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(*result), 10);
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);
    return hpx::util::report_errors();
}
#endif
