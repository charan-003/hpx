//  Copyright (c) 2026 Anshuman Agrawal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file detail/hierarchical_helpers.hpp

#pragma once

#include <hpx/config.hpp>

#include <cstddef>
#include <cstdlib>
#include <vector>

namespace hpx::collectives::detail {

    struct top_level_group
    {
        std::size_t left;
        std::size_t right;
        std::size_t size;
    };

    // Compute the top-level partition of [0, num_sites) into `arity` groups.
    // Uses the same division logic as recursively_fill_communicators:
    // first (num_sites % arity) groups get ceil(num_sites/arity) sites,
    // the rest get floor(num_sites/arity).
    inline std::vector<top_level_group> get_top_level_groups(
        std::size_t num_sites, std::size_t arity)
    {
        if (arity > num_sites)
        {
            arity = num_sites;
        }

        auto const dv = std::lldiv(
            static_cast<long long>(num_sites),
            static_cast<long long>(arity));

        std::vector<top_level_group> groups;
        groups.reserve(arity);

        std::size_t offset = 0;
        for (std::size_t i = 0; i != arity; ++i)
        {
            std::size_t const group_size =
                static_cast<std::size_t>(dv.quot) +
                (i < static_cast<std::size_t>(dv.rem) ? 1 : 0);

            std::size_t const left = offset;
            std::size_t const right = left + group_size - 1;
            groups.push_back(top_level_group{left, right, group_size});

            offset += group_size;
        }

        return groups;
    }

    // Return the index of the top-level group that contains `this_site`.
    inline std::size_t classify_site(
        std::size_t this_site, std::vector<top_level_group> const& groups)
    {
        for (std::size_t i = 0; i != groups.size(); ++i)
        {
            if (this_site >= groups[i].left && this_site <= groups[i].right)
            {
                return i;
            }
        }

        return static_cast<std::size_t>(-1);
    }

    // Return true if `this_site` is the leftmost site (representative)
    // of its top-level group.
    inline bool is_top_level_rep(
        std::size_t this_site, std::size_t num_sites, std::size_t arity)
    {
        auto const groups = get_top_level_groups(num_sites, arity);
        auto const g = classify_site(this_site, groups);
        return (g != static_cast<std::size_t>(-1)) &&
            (this_site == groups[g].left);
    }

}    // namespace hpx::collectives::detail
