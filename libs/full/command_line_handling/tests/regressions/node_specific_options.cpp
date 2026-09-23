//  Copyright (c) 2026 Fabian C.
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/command_line_handling/parse_command_line.hpp>
#include <hpx/modules/ini.hpp>
#include <hpx/modules/program_options.hpp>
#include <hpx/modules/testing.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace {

    hpx::program_options::variables_map parse(hpx::util::section const& rtcfg,
        std::size_t node, std::vector<std::string> const& args)
    {
        hpx::program_options::options_description app_options;
        hpx::program_options::variables_map vm;

        bool const result = hpx::util::parse_commandline(rtcfg, app_options,
            "node_specific_options", args, vm, node,
            hpx::util::commandline_error_mode::rethrow_on_error);

        HPX_TEST(result);
        return vm;
    }

    void test_matching_option(hpx::util::section const& rtcfg)
    {
        auto const vm = parse(rtcfg, 0, {"--hpx:0:pu-offset=16"});

        HPX_TEST_EQ(vm.count("hpx:pu-offset"), std::size_t(1));
        HPX_TEST_EQ(vm["hpx:pu-offset"].as<std::size_t>(), std::size_t(16));
    }

    void test_matching_flag(hpx::util::section const& rtcfg)
    {
        auto const vm = parse(rtcfg, 0, {"--hpx:0:print-bind"});

        HPX_TEST_EQ(vm.count("hpx:print-bind"), std::size_t(1));
    }

    void test_node_specific_bind(hpx::util::section const& rtcfg)
    {
        auto const vm =
            parse(rtcfg, 0, {"--hpx:0:bind=compact", "--hpx:1:bind=scatter"});

        HPX_TEST_EQ(vm.count("hpx:bind"), std::size_t(1));

        auto const& bindings = vm["hpx:bind"].as<std::vector<std::string>>();

        HPX_TEST_EQ(bindings.size(), std::size_t(1));
        HPX_TEST_EQ(bindings[0], std::string("compact"));
    }

    void test_non_matching_options(hpx::util::section const& rtcfg)
    {
        auto const vm =
            parse(rtcfg, 0, {"--hpx:1:pu-offset=16", "--hpx:2:pu-step=2"});

        HPX_TEST_EQ(vm.count("hpx:pu-offset"), std::size_t(0));
        HPX_TEST_EQ(vm.count("hpx:pu-step"), std::size_t(0));
    }

    void test_matching_and_non_matching_options(hpx::util::section const& rtcfg)
    {
        auto const vm = parse(rtcfg, 1,
            {"--hpx:0:pu-offset=8", "--hpx:1:pu-offset=16",
                "--hpx:2:pu-offset=24"});

        HPX_TEST_EQ(vm.count("hpx:pu-offset"), std::size_t(1));
        HPX_TEST_EQ(vm["hpx:pu-offset"].as<std::size_t>(), std::size_t(16));
    }

    void test_unknown_node_ignores_node_specific_options(
        hpx::util::section const& rtcfg)
    {
        auto const vm = parse(rtcfg, static_cast<std::size_t>(-1),
            {"--hpx:0:pu-offset=16", "--hpx:1:print-bind"});

        HPX_TEST_EQ(vm.count("hpx:pu-offset"), std::size_t(0));
        HPX_TEST_EQ(vm.count("hpx:print-bind"), std::size_t(0));
    }
}    // namespace

int main()
{
    hpx::util::section rtcfg;

    test_matching_option(rtcfg);
    test_matching_flag(rtcfg);
    test_node_specific_bind(rtcfg);
    test_non_matching_options(rtcfg);
    test_matching_and_non_matching_options(rtcfg);
    test_unknown_node_ignores_node_specific_options(rtcfg);

    return hpx::util::report_errors();
}
