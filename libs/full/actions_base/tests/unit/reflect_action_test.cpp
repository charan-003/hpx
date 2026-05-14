//  Copyright (c) 2026 Priyanshi Sharma
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#include <hpx/modules/testing.hpp>

#if defined(HPX_SERIALIZATION_HAVE_ALLOW_AUTO_GENERATE)

#include <hpx/actions_base/reflect_action.hpp>

#include <string>

namespace app {

    int compute(double x, double y)
    {
        return (int) (x + y);
    }

    void broadcast(int n) noexcept
    {
        (void) n;
    }

    double transform(float x, int n)
    {
        return x * n;
    }

}    // namespace app

namespace app::nested {

    int deep_compute(int x, int y)
    {
        return x + y;
    }

}    // namespace app::nested

int main()
{
    // Test: action name extraction for simple namespace function
    {
        using compute_action = HPX_ACTION(app::compute);
        HPX_TEST_EQ(std::string(compute_action::get_action_name()),
            std::string("app::compute"));
    }

    // Test: action name extraction for void noexcept function
    {
        using broadcast_action = HPX_ACTION(app::broadcast);
        HPX_TEST_EQ(std::string(broadcast_action::get_action_name()),
            std::string("app::broadcast"));
    }

    // Test: action name extraction for multiple parameters
    {
        using transform_action = HPX_ACTION(app::transform);
        HPX_TEST_EQ(std::string(transform_action::get_action_name()),
            std::string("app::transform"));
    }

    // Test: action name extraction for nested namespace
    {
        using deep_action = HPX_ACTION(app::nested::deep_compute);
        HPX_TEST_EQ(std::string(deep_action::get_action_name()),
            std::string("app::nested::deep_compute"));
    }

    // Test: arity extraction
    {
        using compute_action = HPX_ACTION(app::compute);
        HPX_TEST_EQ(compute_action::arity, std::size_t(2));
    }

    {
        using broadcast_action = HPX_ACTION(app::broadcast);
        HPX_TEST_EQ(broadcast_action::arity, std::size_t(1));
    }

    // Test: function invocation via func_ptr
    {
        using compute_action = HPX_ACTION(app::compute);
        HPX_TEST_EQ(compute_action::func_ptr(3.0, 4.0), 7);
    }

    return hpx::util::report_errors();
}
