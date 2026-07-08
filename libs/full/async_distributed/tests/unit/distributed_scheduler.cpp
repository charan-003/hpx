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
//   5. Standard ex::continues_on routes through distributed_domain.

#include <hpx/config.hpp>
#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/hpx_init.hpp>
#include <hpx/include/runtime.hpp>
#include <hpx/modules/execution.hpp>
#include <hpx/modules/testing.hpp>

#include <hpx/async_distributed/distributed_scheduler.hpp>

#include <atomic>
#include <exception>

namespace ex = hpx::execution::experimental;
namespace tt = hpx::this_thread::experimental;

// We explicitly instantiate and register actions for testing



// Callables for then_t interception tests
struct my_then_callable_1
{
    hpx::tuple<int, hpx::id_type> operator()(int val) const
    {
        return hpx::make_tuple(val * 2, hpx::find_here());
    }
};

struct my_then_callable_2
{
    hpx::tuple<int, hpx::id_type> operator()(int val) const
    {
        return hpx::make_tuple(val + 3, hpx::find_here());
    }
};

///////////////////////////////////////////////////////////////////////////////
int hpx_main()
{
    // Test 1: schedule() on the local locality completes with set_value.
    {
        auto sched = hpx::distributed::experimental::distributed_scheduler{
            hpx::find_here()};
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
        auto s1 = ex::schedule(
            hpx::distributed::experimental::distributed_scheduler(here));

        auto s2 = ex::then(s1, []() { return 42; });
        auto result = tt::sync_wait(s2);

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(*result), 42);
    }

    // Test 3: scheduler equality.
    {
        auto here = hpx::find_here();
        auto s1 = hpx::distributed::experimental::distributed_scheduler{here};
        auto s2 = hpx::distributed::experimental::distributed_scheduler{here};
        HPX_TEST(s1 == s2);
    }

    // Test 5: standard ex::transfer with distributed_scheduler
    //   ex::just(10) | ex::transfer(sched)
    // Test 5: distributed_domain intercepting ex::then
    {
        auto locs = hpx::find_remote_localities();
        auto target = locs.empty() ? hpx::find_here() : locs[0];
        auto sched =
            hpx::distributed::experimental::distributed_scheduler{target};

        auto result = tt::sync_wait(ex::just(10) | ex::continues_on(sched) |
            ex::then(my_then_callable_1{}));

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(std::get<0>(*result)), 20);
        HPX_TEST_EQ(std::get<1>(std::get<0>(*result)), target);
    }

    // Test 6: distributed_domain intercepting ex::then with different value
    {
        auto locs = hpx::find_remote_localities();
        auto target = locs.empty() ? hpx::find_here() : locs[0];
        auto sched =
            hpx::distributed::experimental::distributed_scheduler{target};

        auto result = tt::sync_wait(ex::just(7) | ex::continues_on(sched) |
            ex::then(my_then_callable_2{}));

        HPX_TEST(result.has_value());
        HPX_TEST_EQ(std::get<0>(std::get<0>(*result)), 10);
        HPX_TEST_EQ(std::get<1>(std::get<0>(*result)), target);
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);
    return hpx::util::report_errors();
}
#endif
