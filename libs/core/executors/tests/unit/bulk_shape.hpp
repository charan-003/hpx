//  Copyright (c) 2026 the-ivii
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/modules/execution.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/testing.hpp>

#include <array>
#include <atomic>
#include <forward_list>
#include <ranges>
#include <type_traits>
#include <vector>

namespace executor_test {
    struct void_work
    {
        void operator()(int) const {}
    };

    template <typename T>
    void wait(T&& work)
    {
        if constexpr (hpx::traits::is_future_v<std::decay_t<T>>)
            work.get();
        else if constexpr (hpx::execution::experimental::is_sender_v<
                               std::decay_t<T>>)
            hpx::this_thread::experimental::sync_wait(HPX_FORWARD(T, work));
        else
            hpx::wait_all(work);
    }

    template <typename T>
    std::vector<int> values(T&& work)
    {
        if constexpr (hpx::traits::is_future_v<std::decay_t<T>>)
            return work.get();
        else if constexpr (hpx::execution::experimental::is_sender_v<
                               std::decay_t<T>>)
        {
            auto result =
                hpx::this_thread::experimental::sync_wait(HPX_FORWARD(T, work));
            HPX_TEST(result.has_value());
            return hpx::get<0>(HPX_MOVE(result.value()));
        }
        else
        {
            std::vector<int> result;
            for (auto& future : work)
                result.push_back(future.get());
            return result;
        }
    }

    template <bool ResultContinuation, typename Executor>
    void test_bulk_shape(Executor& exec)
    {
        using forward_shape = std::forward_list<int>;
        static_assert(std::ranges::forward_range<forward_shape>);
        static_assert(!std::ranges::random_access_range<forward_shape>);
        static_assert(!std::ranges::sized_range<forward_shape>);
        static_assert(!requires(forward_shape shape) {
            exec.bulk_async_execute(void_work{}, shape);
        });
        static_assert(!requires(forward_shape shape) {
            exec.bulk_sync_execute(void_work{}, shape);
        });
        static_assert(!requires(
            forward_shape shape, hpx::shared_future<void> predecessor) {
            exec.bulk_then_execute(void_work{}, shape, predecessor);
        });

        std::vector<int> shape{0, 1, 2};
        static_assert(std::ranges::random_access_range<decltype(shape)>);
        static_assert(std::ranges::sized_range<decltype(shape)>);
        std::array<std::atomic<int>, 3> visits{};

        wait(exec.bulk_async_execute(
            [&visits](int value) { ++visits[value]; }, shape));
        for (auto const& count : visits)
            HPX_TEST_EQ(count.load(), 1);

        auto result = values(exec.bulk_async_execute(
            [](int value) { return 2 * value; }, shape));
        HPX_TEST(result == std::vector<int>({0, 2, 4}));

        exec.bulk_sync_execute(
            [&visits](int value) { ++visits[value]; }, shape);
        for (auto const& count : visits)
            HPX_TEST_EQ(count.load(), 2);

        auto predecessor = hpx::make_ready_future().share();
        wait(exec.bulk_then_execute(
            [&visits](int value, hpx::shared_future<void> ready) {
                ready.get();
                ++visits[value];
            },
            shape, predecessor));
        for (auto const& count : visits)
            HPX_TEST_EQ(count.load(), 3);

        if constexpr (ResultContinuation)
        {
            result = values(exec.bulk_then_execute(
                [](int value, hpx::shared_future<void> ready) {
                    ready.get();
                    return 3 * value;
                },
                shape, predecessor));
            HPX_TEST(result == std::vector<int>({0, 3, 6}));
        }

        // The shape may own its elements and die before a sender starts.
        auto work = exec.bulk_async_execute(
            [](int value) { return value; }, std::vector<int>{7, 3, 9});
        result = values(HPX_MOVE(work));
        HPX_TEST(result == std::vector<int>({7, 3, 9}));

        std::vector<int> empty;
        wait(exec.bulk_async_execute([](int) { HPX_TEST(false); }, empty));
    }
}    // namespace executor_test
