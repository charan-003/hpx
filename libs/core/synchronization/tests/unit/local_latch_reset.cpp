//  Copyright (c) 2026 STE||AR Group
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/future.hpp>
#include <hpx/init.hpp>
#include <hpx/latch.hpp>
#include <hpx/modules/testing.hpp>

#include <chrono>

void test_latch_reset_zero_wait_deadlock()
{
    hpx::lcos::local::latch l(1);
    HPX_TEST(!l.try_wait());
    HPX_TEST(!l.is_ready());

    l.count_down(1);
    HPX_TEST(l.try_wait());
    HPX_TEST(l.is_ready());
    l.wait();    // initial wait succeeds

    // Reset the latch with n = 0 (Requires: n >= 0)
    l.reset(0);

    // The counter is 0, so try_wait() and is_ready() return true
    HPX_TEST(l.try_wait());
    HPX_TEST(l.is_ready());

    // According to latch specification:
    // "If counter_ is 0, returns immediately. Otherwise, blocks the calling
    //  thread at the synchronization point until counter_ reaches 0."
    //
    // However, reset(0) sets counter_ = 0 but leaves notified_ = false.
    // In wait():
    //   while (counter_.load(std::memory_order_acquire) > 0 || !notified_)
    //       cond_.data_.wait(l, "hpx::latch::wait");
    //
    // Since !notified_ is true, wait() unconditionally deadlocks on cond_.wait().
    // We launch wait() in an asynchronous task and check for completion.
    hpx::future<void> f = hpx::async([&l] { l.wait(); });

    hpx::future_status const status =
        f.wait_for(std::chrono::milliseconds(500));

    // This assertion MUST FAIL on current master because l.wait() deadlocks!
    HPX_TEST_MSG(status == hpx::future_status::ready,
        "DEADLOCK DETECTED: l.wait() blocked indefinitely after reset(0) "
        "despite counter_ == 0 and is_ready() == true");

    if (status != hpx::future_status::ready)
    {
        // Unblock the deadlocked thread so test runner can complete and report failure
        l.abort_all();
        try
        {
            f.get();
        }
        catch (...)
        {
            // Suppress exception to allow test cleanup.
        }
    }
}

void test_latch_reset_zero_arrive_and_wait_deadlock()
{
    hpx::lcos::local::latch l(1);
    l.count_down(1);
    l.wait();

    l.reset(0);
    HPX_TEST(l.is_ready());

    hpx::future<void> f = hpx::async([&l] { l.arrive_and_wait(0); });

    hpx::future_status const status =
        f.wait_for(std::chrono::milliseconds(500));

    HPX_TEST_MSG(status == hpx::future_status::ready,
        "DEADLOCK DETECTED: l.arrive_and_wait(0) blocked indefinitely after "
        "reset(0)");

    if (status != hpx::future_status::ready)
    {
        l.abort_all();
        try
        {
            f.get();
        }
        catch (...)
        {
            // Suppress exception to allow test cleanup.
        }
    }
}

int hpx_main()
{
    test_latch_reset_zero_wait_deadlock();
    test_latch_reset_zero_arrive_and_wait_deadlock();

    HPX_TEST_EQ(hpx::local::finalize(), 0);
    return 0;
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ_MSG(hpx::local::init(hpx_main, argc, argv), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
