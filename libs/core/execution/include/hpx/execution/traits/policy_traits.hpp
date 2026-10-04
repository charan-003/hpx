//  Copyright (c) 2026 Rohan Pattanayak
//  Copyright (c) 2026 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file hpx/execution/traits/policy_traits.hpp

#pragma once

#include <hpx/config.hpp>

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace hpx::execution {

    namespace detail {

        /// \brief Default value for every policy_traits member.
        ///
        /// policy_traits itself, and every specialization of policy_traits,
        /// derives from policy_traits_default, so a specialization only needs
        /// to declare the members that differ from the default; every member it
        /// does not redeclare is inherited as false. This also means a new
        /// member added here is picked up by every existing specialization
        /// automatically, instead of silently being left undefined until each
        /// specialization is updated by hand.
        HPX_CXX_CORE_EXPORT struct policy_traits_default
        {
            /// Whether Policy is a recognized HPX execution policy.
            static constexpr bool is_policy = false;

            /// Whether Policy was produced by rebinding an executor and/or a
            /// set of executor parameters onto another execution policy.
            static constexpr bool is_rebound = false;

            /// Whether Policy permits its algorithm to run in parallel across
            /// more than one execution agent.
            static constexpr bool is_parallel = false;

            /// Whether Policy requires its algorithm to run on a single
            /// execution agent, without parallelization.
            static constexpr bool is_sequenced = false;

            /// Whether Policy permits its algorithm to be vectorized.
            static constexpr bool is_unsequenced = false;

            /// Whether Policy runs its algorithm asynchronously, returning a
            /// future rather than blocking the calling thread.
            static constexpr bool is_async = false;

            /// Whether Policy operates on vector packs rather than individual
            /// elements.
            static constexpr bool is_vectorpack = false;

#if defined(HPX_HAVE_DATAPAR)
            // Datapar policies expose the number of vector lanes they cover
            static constexpr std::size_t num_lanes = 0;
#endif
        };
    }    // namespace detail

#define HPX_DEFINE_POLICY_TRAIT(name)                                          \
    static constexpr bool name = []() {                                        \
        if constexpr (requires {                                               \
                          { Policy::name } -> std::convertible_to<bool>;       \
                      })                                                       \
        {                                                                      \
            return Policy::name;                                               \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
    }() /**/

    /// \brief Canonical description of the properties of a concrete execution
    ///        policy type.
    ///
    /// A new execution policy is made known to the is_execution_policy,
    /// is_parallel_execution_policy, is_sequenced_execution_policy,
    /// is_unsequenced_execution_policy, is_async_execution_policy,
    /// is_rebound_execution_policy, and is_vectorpack_execution_policy
    /// customization points by specializing policy_traits for that policy's
    /// type, deriving the specialization from policy_traits_default, and
    /// setting only the members that are true, instead of specializing each of
    /// those traits individually.
    ///
    /// The execution policies will provide the concrete traits that evaluate to
    /// true.
    HPX_CXX_CORE_EXPORT template <typename Policy>
    struct policy_traits
    {
        /// Whether Policy is a recognized HPX execution policy.
        HPX_DEFINE_POLICY_TRAIT(is_policy);

        /// Whether Policy was produced by rebinding an executor and/or a
        /// set of executor parameters onto another execution policy.
        HPX_DEFINE_POLICY_TRAIT(is_rebound);

        /// Whether Policy permits its algorithm to run in parallel across
        /// more than one execution agent.
        HPX_DEFINE_POLICY_TRAIT(is_parallel);

        /// Whether Policy requires its algorithm to run on a single
        /// execution agent, without parallelization.
        HPX_DEFINE_POLICY_TRAIT(is_sequenced);

        /// Whether Policy permits its algorithm to be vectorized.
        HPX_DEFINE_POLICY_TRAIT(is_unsequenced);

        /// Whether Policy runs its algorithm asynchronously, returning a
        /// future rather than blocking the calling thread.
        HPX_DEFINE_POLICY_TRAIT(is_async);

        /// Whether Policy operates on vector packs rather than individual
        /// elements.
        HPX_DEFINE_POLICY_TRAIT(is_vectorpack);

#if defined(HPX_HAVE_DATAPAR)
        // Datapar policies expose the number of vector lanes they cover
        static constexpr std::size_t num_lanes = []() {
            if constexpr (requires {
                              {
                                  Policy::num_lanes
                              } -> std::convertible_to<std::size_t>;
                          })
            {
                return Policy::num_lanes;
            }
            else
            {
                return 0;
            }
        }();
#endif
    };

#undef HPX_DEFINE_POLICY_TRAIT
}    // namespace hpx::execution
