//  Copyright (c) 2026 Anshuman Agrawal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/assert.hpp>
#include <hpx/collectives/create_communicator.hpp>
#include <hpx/collectives/detail/hierarchical_helpers.hpp>
#include <hpx/modules/errors.hpp>

#include <cstddef>
#include <exception>

namespace hpx::collectives::detail {

    std::exception_ptr validate_hierarchical_communicator(
        hierarchical_communicator const& communicators,
        this_site_arg const this_site, char const* name)
    {
        if (!communicators.valid())
        {
            return HPX_GET_EXCEPTION(hpx::error::invalid_status, name,
                "the hierarchical communicator is not valid");
        }

        auto const [num_sites_val, communicator_site] =
            communicators.get_info();

        if (this_site >= num_sites_val)
        {
            return HPX_GET_EXCEPTION(hpx::error::bad_parameter, name,
                "this_site must be smaller than the number of participating "
                "sites");
        }

        if (this_site != communicator_site)
        {
            return HPX_GET_EXCEPTION(hpx::error::bad_parameter, name,
                "this_site must match the site used to create the "
                "hierarchical communicator");
        }

        return std::exception_ptr{};
    }

    std::size_t get_top_level_group_count(
        std::size_t const num_sites, std::size_t const arity)
    {
        HPX_ASSERT(num_sites != 0);
        HPX_ASSERT(arity != 0);

        return arity > num_sites ? num_sites : arity;
    }

    std::size_t get_top_level_group_size(std::size_t const group,
        std::size_t const num_sites, std::size_t const arity)
    {
        std::size_t const count = get_top_level_group_count(num_sites, arity);
        HPX_ASSERT(group < count);

        std::size_t const division_steps = num_sites / count;
        std::size_t const remainder = num_sites % count;

        return division_steps + (group < remainder ? 1 : 0);
    }

    std::size_t get_top_level_group_left(std::size_t const group,
        std::size_t const num_sites, std::size_t const arity)
    {
        std::size_t const count = get_top_level_group_count(num_sites, arity);
        HPX_ASSERT(group < count);

        std::size_t const division_steps = num_sites / count;
        std::size_t const remainder = num_sites % count;

        return group * division_steps + (group < remainder ? group : remainder);
    }

    std::ptrdiff_t classify_site(std::size_t const this_site,
        std::size_t const num_sites, std::size_t const arity)
    {
        if (this_site >= num_sites)
        {
            return -1;
        }

        std::size_t const count = get_top_level_group_count(num_sites, arity);
        for (std::size_t group = 0; group != count; ++group)
        {
            std::size_t const left =
                get_top_level_group_left(group, num_sites, arity);
            std::size_t const size =
                get_top_level_group_size(group, num_sites, arity);

            if (this_site >= left && this_site < left + size)
            {
                return static_cast<std::ptrdiff_t>(group);
            }
        }

        return -1;
    }

    bool is_top_level_rep(std::size_t const this_site,
        std::size_t const num_sites, std::size_t const arity)
    {
        std::ptrdiff_t const group = classify_site(this_site, num_sites, arity);
        if (group == -1)
        {
            return false;
        }

        return this_site ==
            get_top_level_group_left(
                static_cast<std::size_t>(group), num_sites, arity);
    }
}    // namespace hpx::collectives::detail

#endif
