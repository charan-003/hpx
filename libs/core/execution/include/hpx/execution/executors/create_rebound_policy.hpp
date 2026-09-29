//  Copyright (c) 2016-2024 Hartmut Kaiser
//  Copyright (c) 2026 Rohan Pattanayak
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file hpx/execution/executors/create_rebound_policy.hpp
///
/// \brief hpx::execution::experimental::create_rebound_policy, the
///        customization point that rebinds an execution policy's executor
///        and/or its executor parameters and constructs the result, and the
///        per-axis construction customization points it dispatches to.
///
/// This lives in its own header, separate from both rebind_executor.hpp
/// and rebind_policy.hpp: the construction side depends on
/// hpx::execution::detail::rebind_policy_executor_t and
/// rebind_policy_parameters_t (see rebind_policy.hpp), and
/// rebind_policy.hpp already includes rebind_executor.hpp, so defining
/// create_rebound_policy_t in either of those two headers would make them
/// include each other; this header depends on both instead.

#pragma once

#include <hpx/config.hpp>
#include <hpx/execution/executors/rebind_executor.hpp>
#include <hpx/execution/executors/rebind_policy.hpp>
#include <hpx/modules/execution_base.hpp>

#include <type_traits>
#include <utility>

namespace hpx::execution::experimental {

    /// \brief Customization point controlling how an execution policy
    ///        rebound to a new executor is constructed from the original
    ///        policy.
    ///
    /// hpx::execution::detail::rebind_policy_executor only computes the
    /// rebound type. A policy that carries additional state, or that cannot
    /// be constructed from just (executor, parameters), specializes this
    /// template to build the rebound policy itself; \c call receives the
    /// original policy, so any such state can be carried over. The result
    /// must be of type
    /// hpx::execution::detail::rebind_policy_executor_t<Policy, Executor>.
    ///
    /// The default constructs the rebound policy from the new executor and
    /// the original policy's parameters().
    ///
    /// \tparam Policy   The (decayed) execution policy type being rebound.
    /// \tparam Executor The (decayed) executor type Policy is rebound to.
    HPX_CXX_CORE_EXPORT template <typename Policy, typename Executor>
    struct construct_rebound_policy_executor
    {
        template <typename Executor_>
        static constexpr hpx::execution::detail::rebind_policy_executor_t<
            Policy, Executor>
        call(Policy const& policy, Executor_&& exec)
        {
            return hpx::execution::detail::rebind_policy_executor_t<Policy,
                Executor>(HPX_FORWARD(Executor_, exec), policy.parameters());
        }
    };

    /// \brief Customization point controlling how an execution policy
    ///        rebound to new executor parameters is constructed from the
    ///        original policy.
    ///
    /// Mirrors construct_rebound_policy_executor along the parameters
    /// axis. The result must be of type
    /// hpx::execution::detail::rebind_policy_parameters_t<Policy, Parameters>.
    ///
    /// The default constructs the rebound policy from the original
    /// policy's executor() and the new parameters.
    ///
    /// \tparam Policy     The (decayed) execution policy type being rebound.
    /// \tparam Parameters The (decayed) executor parameters type Policy is
    ///                    rebound to.
    HPX_CXX_CORE_EXPORT template <typename Policy, typename Parameters>
    struct construct_rebound_policy_parameters
    {
        template <typename Parameters_>
        static constexpr hpx::execution::detail::rebind_policy_parameters_t<
            Policy, Parameters>
        call(Policy const& policy, Parameters_&& parameters)
        {
            return hpx::execution::detail::rebind_policy_parameters_t<Policy,
                Parameters>(
                policy.executor(), HPX_FORWARD(Parameters_, parameters));
        }
    };

    /// \brief Rebind the executor of \a policy to \a exec and construct the
    ///        result through construct_rebound_policy_executor.
    HPX_CXX_CORE_EXPORT inline constexpr struct
        create_rebound_policy_executor_t final
    {
        template <typename Policy, typename Executor>
            requires(hpx::executor_any<Executor>)
        constexpr decltype(auto) operator()(
            Policy&& policy, Executor&& exec) const
        {
            using rebound_type =
                hpx::execution::detail::rebind_policy_executor_t<Policy,
                    Executor>;
            using construct_type =
                construct_rebound_policy_executor<std::decay_t<Policy>,
                    std::decay_t<Executor>>;
            using result_type = decltype(construct_type::call(
                policy, HPX_FORWARD(Executor, exec)));

            static_assert(
                std::is_same_v<std::decay_t<result_type>, rebound_type>,
                "construct_rebound_policy_executor must produce "
                "hpx::execution::detail::rebind_policy_executor_t<Policy, "
                "Executor>");

            return construct_type::call(policy, HPX_FORWARD(Executor, exec));
        }
    } create_rebound_policy_executor{};

    /// \brief Rebind the executor parameters of \a policy to \a parameters
    ///        and construct the result through
    ///        construct_rebound_policy_parameters.
    HPX_CXX_CORE_EXPORT inline constexpr struct
        create_rebound_policy_parameters_t final
    {
        template <typename Policy, typename Parameters>
            requires(hpx::executor_parameters<Parameters>)
        constexpr decltype(auto) operator()(
            Policy&& policy, Parameters&& parameters) const
        {
            using rebound_type =
                hpx::execution::detail::rebind_policy_parameters_t<Policy,
                    Parameters>;
            using construct_type =
                construct_rebound_policy_parameters<std::decay_t<Policy>,
                    std::decay_t<Parameters>>;
            using result_type = decltype(construct_type::call(
                policy, HPX_FORWARD(Parameters, parameters)));

            static_assert(
                std::is_same_v<std::decay_t<result_type>, rebound_type>,
                "construct_rebound_policy_parameters must produce "
                "hpx::execution::detail::rebind_policy_parameters_t<Policy, "
                "Parameters>");

            return construct_type::call(
                policy, HPX_FORWARD(Parameters, parameters));
        }
    } create_rebound_policy_parameters{};

    //////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT inline constexpr struct create_rebound_policy_t final
    {
        /// \brief Rebind both the executor and the executor parameters of
        ///        \a policy in one step, and construct the result from
        ///        \a exec and \a parameters.
        ///
        /// Goes through the combined rebind_executor_t: there is no
        /// combined construction customization point, and the original
        /// policy contributes nothing to the result when both axes are
        /// replaced.
        template <typename ExPolicy, typename Executor, typename Parameters>
            requires(hpx::executor_any<Executor> &&
                hpx::executor_parameters<Parameters>)
        constexpr decltype(auto) operator()(
            ExPolicy&&, Executor&& exec, Parameters&& parameters) const
        {
            using rebound_type =
                rebind_executor_t<ExPolicy, Executor, Parameters>;

            return rebound_type(HPX_FORWARD(Executor, exec),
                HPX_FORWARD(Parameters, parameters));
        }

        /// \brief Rebind only the executor of \a policy, keeping its
        ///        executor parameters, and construct the result.
        ///
        /// Dispatches through create_rebound_policy_executor, so both a
        /// hpx::execution::detail::rebind_policy_executor and a
        /// construct_rebound_policy_executor specialization for the policy
        /// are honored.
        template <typename ExPolicy, typename Executor>
            requires(hpx::executor_any<Executor>)
        constexpr decltype(auto) operator()(
            ExPolicy&& policy, Executor&& exec) const
        {
            return create_rebound_policy_executor(
                HPX_FORWARD(ExPolicy, policy), HPX_FORWARD(Executor, exec));
        }

        /// \brief Rebind only the executor parameters of \a policy,
        ///        keeping its executor, and construct the result.
        ///
        /// Mirrors the executor-only overload above along the other axis,
        /// dispatching through create_rebound_policy_parameters.
        template <typename ExPolicy, typename Parameters>
            requires(hpx::executor_parameters<Parameters>)
        constexpr decltype(auto) operator()(
            ExPolicy&& policy, Parameters&& parameters) const
        {
            return create_rebound_policy_parameters(
                HPX_FORWARD(ExPolicy, policy),
                HPX_FORWARD(Parameters, parameters));
        }
    } create_rebound_policy{};
}    // namespace hpx::execution::experimental
