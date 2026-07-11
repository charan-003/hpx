//  Copyright (c) 2026 Anshuman Agrawal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/collectives/detail/hierarchical_helpers.hpp>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/modules/testing.hpp>

#include <hpx/modules/collectives.hpp>

#include <cstddef>
#include <string>
#include <vector>

using hpx::collectives::detail::classify_site;
using hpx::collectives::detail::get_top_level_group_count;
using hpx::collectives::detail::get_top_level_group_left;
using hpx::collectives::detail::get_top_level_group_size;
using hpx::collectives::detail::is_top_level_rep;

std::size_t expected_group_count(std::size_t const n, std::size_t const arity)
{
    return arity > n ? n : arity;
}

std::size_t expected_group_size(
    std::size_t const group, std::size_t const n, std::size_t const arity)
{
    std::size_t const count = expected_group_count(n, arity);
    std::size_t const division_steps = n / count;
    std::size_t const remainder = n % count;

    return division_steps + (group < remainder ? 1 : 0);
}

std::size_t expected_group_left(
    std::size_t const group, std::size_t const n, std::size_t const arity)
{
    std::size_t const count = expected_group_count(n, arity);
    std::size_t const division_steps = n / count;
    std::size_t const remainder = n % count;

    return group * division_steps + (group < remainder ? group : remainder);
}

std::size_t expected_group_right(
    std::size_t const group, std::size_t const n, std::size_t const arity)
{
    return expected_group_left(group, n, arity) +
        expected_group_size(group, n, arity) - 1;
}

std::ptrdiff_t expected_classification(
    std::size_t const site, std::size_t const n, std::size_t const arity)
{
    if (site >= n)
    {
        return -1;
    }

    std::size_t const count = expected_group_count(n, arity);
    for (std::size_t group = 0; group != count; ++group)
    {
        if (site >= expected_group_left(group, n, arity) &&
            site <= expected_group_right(group, n, arity))
        {
            return static_cast<std::ptrdiff_t>(group);
        }
    }

    return -1;
}

void check_group(std::size_t const group, std::size_t const n,
    std::size_t const arity, std::size_t const left, std::size_t const right,
    std::size_t const size)
{
    HPX_TEST_EQ(get_top_level_group_left(group, n, arity), left);
    HPX_TEST_EQ(get_top_level_group_size(group, n, arity), size);
    HPX_TEST_EQ(left + size - 1, right);
}

void test_balanced_arity2()
{
    // N=8, arity=2 -> two groups of 4: [0,3] and [4,7]
    HPX_TEST_EQ(get_top_level_group_count(8, 2), static_cast<std::size_t>(2));

    check_group(0, 8, 2, 0, 3, 4);
    check_group(1, 8, 2, 4, 7, 4);
}

void test_balanced_arity4()
{
    // N=8, arity=4 -> four groups of 2: [0,1], [2,3], [4,5], [6,7]
    HPX_TEST_EQ(get_top_level_group_count(8, 4), static_cast<std::size_t>(4));

    for (std::size_t i = 0; i != 4; ++i)
    {
        check_group(i, 8, 4, i * 2, i * 2 + 1, 2);
    }
}

void test_unbalanced_n11_arity4()
{
    // N=11, arity=4 -> 11/4=2 rem 3
    // groups: [0,2](3), [3,5](3), [6,8](3), [9,10](2)
    HPX_TEST_EQ(get_top_level_group_count(11, 4), static_cast<std::size_t>(4));

    check_group(0, 11, 4, 0, 2, 3);
    check_group(1, 11, 4, 3, 5, 3);
    check_group(2, 11, 4, 6, 8, 3);
    check_group(3, 11, 4, 9, 10, 2);
}

void test_unbalanced_n7_arity3()
{
    // N=7, arity=3 -> 7/3=2 rem 1
    // groups: [0,2](3), [3,4](2), [5,6](2)
    HPX_TEST_EQ(get_top_level_group_count(7, 3), static_cast<std::size_t>(3));

    check_group(0, 7, 3, 0, 2, 3);
    check_group(1, 7, 3, 3, 4, 2);
    check_group(2, 7, 3, 5, 6, 2);
}

void test_arity_exceeds_n()
{
    // N=3, arity=8 -> clamp arity to 3, three groups of 1
    HPX_TEST_EQ(get_top_level_group_count(3, 8), static_cast<std::size_t>(3));

    for (std::size_t i = 0; i != 3; ++i)
    {
        check_group(i, 3, 8, i, i, 1);
    }
}

void test_single_site()
{
    // N=1, arity=2 -> one group of 1
    HPX_TEST_EQ(get_top_level_group_count(1, 2), static_cast<std::size_t>(1));

    check_group(0, 1, 2, 0, 0, 1);
}

void test_coverage_property()
{
    // For various N and arity, verify that the groups cover [0, N) exactly.
    for (std::size_t n = 1; n <= 32; ++n)
    {
        for (std::size_t arity = 2; arity <= 8; ++arity)
        {
            std::size_t const count = get_top_level_group_count(n, arity);
            std::size_t total_size = 0;

            for (std::size_t group = 0; group != count; ++group)
            {
                std::size_t const left =
                    get_top_level_group_left(group, n, arity);
                std::size_t const size =
                    get_top_level_group_size(group, n, arity);

                total_size += size;

                if (group == 0)
                {
                    HPX_TEST_EQ(left, static_cast<std::size_t>(0));
                }
                else
                {
                    std::size_t const prev_left =
                        get_top_level_group_left(group - 1, n, arity);
                    std::size_t const prev_size =
                        get_top_level_group_size(group - 1, n, arity);
                    HPX_TEST_EQ(left, prev_left + prev_size);
                }

                if (group == count - 1)
                {
                    HPX_TEST_EQ(left + size - 1, n - 1);
                }
            }

            HPX_TEST_EQ(total_size, n);
        }
    }
}

void test_classify_site_basic()
{
    HPX_TEST_EQ(classify_site(0, 11, 4), static_cast<std::ptrdiff_t>(0));
    HPX_TEST_EQ(classify_site(1, 11, 4), static_cast<std::ptrdiff_t>(0));
    HPX_TEST_EQ(classify_site(2, 11, 4), static_cast<std::ptrdiff_t>(0));
    HPX_TEST_EQ(classify_site(3, 11, 4), static_cast<std::ptrdiff_t>(1));
    HPX_TEST_EQ(classify_site(5, 11, 4), static_cast<std::ptrdiff_t>(1));
    HPX_TEST_EQ(classify_site(6, 11, 4), static_cast<std::ptrdiff_t>(2));
    HPX_TEST_EQ(classify_site(9, 11, 4), static_cast<std::ptrdiff_t>(3));
    HPX_TEST_EQ(classify_site(10, 11, 4), static_cast<std::ptrdiff_t>(3));
    HPX_TEST_EQ(classify_site(11, 11, 4), static_cast<std::ptrdiff_t>(-1));
}

void test_classify_site_exhaustive()
{
    // Every site in [0, N) must classify to exactly one group.
    for (std::size_t n = 1; n <= 32; ++n)
    {
        for (std::size_t arity = 2; arity <= 8; ++arity)
        {
            for (std::size_t site = 0; site != n; ++site)
            {
                HPX_TEST_EQ(classify_site(site, n, arity),
                    expected_classification(site, n, arity));
            }
        }
    }
}

void test_matches_recursive_fill()
{
    // Verify that the top-level helpers produce the same partition as the top
    // frame of recursively_fill_communicators.
    for (std::size_t n = 1; n <= 32; ++n)
    {
        for (std::size_t arity = 2; arity <= 8; ++arity)
        {
            std::size_t const count = get_top_level_group_count(n, arity);
            HPX_TEST_EQ(count, expected_group_count(n, arity));

            for (std::size_t group = 0; group != count; ++group)
            {
                HPX_TEST_EQ(get_top_level_group_left(group, n, arity),
                    expected_group_left(group, n, arity));
                HPX_TEST_EQ(get_top_level_group_size(group, n, arity),
                    expected_group_size(group, n, arity));
            }
        }
    }
}

void test_is_top_level_rep_basic()
{
    // N=8, arity=2 -> groups [0,3] and [4,7]. Reps are 0 and 4.
    HPX_TEST(is_top_level_rep(0, 8, 2));
    HPX_TEST(!is_top_level_rep(1, 8, 2));
    HPX_TEST(!is_top_level_rep(3, 8, 2));
    HPX_TEST(is_top_level_rep(4, 8, 2));
    HPX_TEST(!is_top_level_rep(5, 8, 2));
    HPX_TEST(!is_top_level_rep(7, 8, 2));

    // N=11, arity=4 -> groups [0,2], [3,5], [6,8], [9,10]. Reps: 0,3,6,9.
    HPX_TEST(is_top_level_rep(0, 11, 4));
    HPX_TEST(!is_top_level_rep(1, 11, 4));
    HPX_TEST(!is_top_level_rep(2, 11, 4));
    HPX_TEST(is_top_level_rep(3, 11, 4));
    HPX_TEST(is_top_level_rep(6, 11, 4));
    HPX_TEST(is_top_level_rep(9, 11, 4));
    HPX_TEST(!is_top_level_rep(10, 11, 4));
}

void test_is_top_level_rep_exhaustive()
{
    // For various N and arity, exactly the leftmost site in each group
    // should be a rep, and no other site.
    for (std::size_t n = 1; n <= 32; ++n)
    {
        for (std::size_t arity = 2; arity <= 8; ++arity)
        {
            for (std::size_t site = 0; site != n; ++site)
            {
                std::ptrdiff_t const group =
                    expected_classification(site, n, arity);
                bool const expected = group != -1 &&
                    site ==
                        expected_group_left(
                            static_cast<std::size_t>(group), n, arity);

                HPX_TEST_EQ(is_top_level_rep(site, n, arity), expected);
            }
        }
    }
}

int hpx_main()
{
    test_balanced_arity2();
    test_balanced_arity4();
    test_unbalanced_n11_arity4();
    test_unbalanced_n7_arity3();
    test_arity_exceeds_n();
    test_single_site();
    test_coverage_property();
    test_classify_site_basic();
    test_classify_site_exhaustive();
    test_matches_recursive_fill();
    test_is_top_level_rep_basic();
    test_is_top_level_rep_exhaustive();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> const cfg = {"hpx.run_hpx_main!=1"};

    hpx::init_params init_args;
    init_args.cfg = cfg;

    HPX_TEST_EQ(hpx::init(argc, argv, init_args), 0);
    return hpx::util::report_errors();
}

#endif
