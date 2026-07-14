//  Copyright (c) 2019-2026 Hartmut Kaiser
//  Copyright (c) 2026 Anshuman Agrawal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file all_to_all.hpp

#pragma once

#if defined(DOXYGEN)
// clang-format off
namespace hpx { namespace collectives {

    /// AllToAll a set of values from different call sites
    ///
    /// This function receives a set of values from all call sites operating on
    /// the given base name.
    ///
    /// \param basename     The base name identifying the all_to_all operation
    /// \param local_result A vector of values to transmit to all
    ///                     participating sites from this call site.
    /// \param num_sites    The number of participating sites (default: all
    ///                     localities).
    /// \param this_site    The sequence number of this invocation (usually
    ///                     the locality id). This value is optional and
    ///                     defaults to whatever hpx::get_locality_id() returns.
    /// \param generation   The generational counter identifying the sequence
    ///                     number of the all_to_all operation performed on the
    ///                     given base name. This is optional and needs to be
    ///                     supplied only if the all_to_all operation on the
    ///                     given base name has to be performed more than once.
    ///                     The generation number (if given) must be a positive
    ///                     number greater than zero.
    /// \param root_site    The site that is responsible for creating the
    ///                     all_to_all support object. This value is optional
    ///                     and defaults to '0' (zero).
    ///
    /// \returns    This function returns a future holding a vector with all
    ///             values send by all participating sites. It will become
    ///             ready once the all_to_all operation has been completed.
    ///
    template <typename T> hpx::future<std::vector<T>>
    all_to_all(char const* basename,
        std::vector<T>&& local_result,
        num_sites_arg num_sites = num_sites_arg(),
        this_site_arg this_site = this_site_arg(),
        generation_arg generation = generation_arg(),
        root_site_arg root_site = root_site_arg());

    /// AllToAll a set of values from different call sites
    ///
    /// This function receives a set of values from all call sites operating on
    /// the given base name.
    ///
    /// \param fid          A communicator object returned from \a create_communicator
    /// \param local_result A vector of values to transmit to all
    ///                     participating sites from this call site.
    /// \param this_site    The sequence number of this invocation (usually
    ///                     the locality id). This value is optional and
    ///                     defaults to whatever hpx::get_locality_id() returns.
    /// \param generation   The generational counter identifying the sequence
    ///                     number of the all_to_all operation performed on the
    ///                     given base name. This is optional and needs to be
    ///                     supplied only if the all_to_all operation on the
    ///                     given base name has to be performed more than once.
    ///                     The generation number (if given) must be a positive
    ///                     number greater than zero.
    ///
    /// \returns    This function returns a future holding a vector with all
    ///             values send by all participating sites. It will become
    ///             ready once the all_to_all operation has been completed.
    ///
    template <typename T> hpx::future<std::vector<T>>
    all_to_all(communicator fid,
        std::vector<T>&& local_result,
        this_site_arg this_site = this_site_arg(),
        generation_arg generation = generation_arg());

    /// AllToAll a set of values from different call sites
    ///
    /// This function receives a set of values from all call sites operating on
    /// the given base name.
    ///
    /// \param fid          A communicator object returned from \a create_communicator
    /// \param local_result A vector of values to transmit to all
    ///                     participating sites from this call site.
    /// \param generation   The generational counter identifying the sequence
    ///                     number of the all_to_all operation performed on the
    ///                     given base name. This is optional and needs to be
    ///                     supplied only if the all_to_all operation on the
    ///                     given base name has to be performed more than once.
    ///                     The generation number (if given) must be a positive
    ///                     number greater than zero.
    /// \param this_site    The sequence number of this invocation (usually
    ///                     the locality id). This value is optional and
    ///                     defaults to whatever hpx::get_locality_id() returns.
    ///
    /// \returns    This function returns a future holding a vector with all
    ///             values send by all participating sites. It will become
    ///             ready once the all_to_all operation has been completed.
    ///
    template <typename T> hpx::future<std::vector<T>>
    all_to_all(communicator fid,
        std::vector<T>&& local_result,
        generation_arg generation,
        this_site_arg this_site = this_site_arg());

    /// AllToAll a set of values from different call sites
    ///
    /// This function receives a set of values from all call sites operating on
    /// the given base name.
    ///
    /// \param policy       The execution policy specifying synchronous execution.
    /// \param basename     The base name identifying the all_to_all operation
    /// \param local_result A vector of values to transmit to all
    ///                     participating sites from this call site.
    /// \param num_sites    The number of participating sites (default: all
    ///                     localities).
    /// \param this_site    The sequence number of this invocation (usually
    ///                     the locality id). This value is optional and
    ///                     defaults to whatever hpx::get_locality_id() returns.
    /// \param generation   The generational counter identifying the sequence
    ///                     number of the all_to_all operation performed on the
    ///                     given base name. This is optional and needs to be
    ///                     supplied only if the all_to_all operation on the
    ///                     given base name has to be performed more than once.
    ///                     The generation number (if given) must be a positive
    ///                     number greater than zero.
    /// \param root_site    The site that is responsible for creating the
    ///                     all_to_all support object. This value is optional
    ///                     and defaults to '0' (zero).
    ///
    /// \returns    This function returns a vector with all values send by all
    ///             participating sites. This function executes synchronously and
    ///             directly returns the result.
    ///
    template <typename T>
    std::vector<T> all_to_all(hpx::launch::sync_policy policy,
        char const* basename, std::vector<T>&& local_result,
        num_sites_arg num_sites = num_sites_arg(),
        this_site_arg this_site = this_site_arg(),
        generation_arg generation = generation_arg(),
        root_site_arg root_site = root_site_arg());

    /// AllToAll a set of values from different call sites
    ///
    /// This function receives a set of values from all call sites operating on
    /// the given base name.
    ///
    /// \param  policy      The execution policy specifying synchronous execution.
    /// \param fid          A communicator object returned from \a create_communicator
    /// \param local_result A vector of values to transmit to all
    ///                     participating sites from this call site.
    /// \param this_site    The sequence number of this invocation (usually
    ///                     the locality id). This value is optional and
    ///                     defaults to whatever hpx::get_locality_id() returns.
    /// \param generation   The generational counter identifying the sequence
    ///                     number of the all_to_all operation performed on the
    ///                     given base name. This is optional and needs to be
    ///                     supplied only if the all_to_all operation on the
    ///                     given base name has to be performed more than once.
    ///                     The generation number (if given) must be a positive
    ///                     number greater than zero.
    ///
    /// \returns    This function returns a vector with all values send by all
    ///             participating sites. This function executes synchronously and
    ///             directly returns the result.
    ///
    template <typename T>
    std::vector<T> all_to_all(hpx::launch::sync_policy policy,
        communicator fid, std::vector<T>&& local_result,
        this_site_arg this_site = this_site_arg(),
        generation_arg generation = generation_arg());

    /// AllToAll a set of values from different call sites
    ///
    /// This function receives a set of values from all call sites operating on
    /// the given base name.
    ///
    /// \param policy      The execution policy specifying synchronous execution.
    /// \param fid          A communicator object returned from \a create_communicator
    /// \param local_result A vector of values to transmit to all
    ///                     participating sites from this call site.
    /// \param generation   The generational counter identifying the sequence
    ///                     number of the all_to_all operation performed on the
    ///                     given base name. This is optional and needs to be
    ///                     supplied only if the all_to_all operation on the
    ///                     given base name has to be performed more than once.
    ///                     The generation number (if given) must be a positive
    ///                     number greater than zero.
    /// \param this_site    The sequence number of this invocation (usually
    ///                     the locality id). This value is optional and
    ///                     defaults to whatever hpx::get_locality_id() returns.
    ///
    /// \returns    This function returns a vector with all values send by all
    ///             participating sites. This function executes synchronously and
    ///             directly returns the result.
    ///
    template <typename T>
    std::vector<T> all_to_all(hpx::launch::sync_policy policy,
        communicator fid, std::vector<T>&& local_result,
        generation_arg generation, this_site_arg this_site = this_site_arg());
}}    // namespace hpx::collectives

// clang-format on
#else

#include <hpx/config.hpp>

#if !defined(HPX_COMPUTE_DEVICE_CODE)

#include <hpx/assert.hpp>
#include <hpx/collectives/argument_types.hpp>
#include <hpx/collectives/create_communicator.hpp>
#include <hpx/collectives/detail/flattened_data.hpp>
#include <hpx/collectives/detail/hierarchical_all_to_all_helpers.hpp>
#include <hpx/collectives/detail/hierarchical_helpers.hpp>
#include <hpx/modules/async_base.hpp>
#include <hpx/modules/async_distributed.hpp>
#include <hpx/modules/components_base.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/futures.hpp>
#include <hpx/modules/type_support.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace hpx::traits {

    namespace communication {

        HPX_CXX_EXPORT struct all_to_all_tag;

        template <>
        struct communicator_data<all_to_all_tag>
        {
            HPX_EXPORT static char const* name() noexcept;
        };
    }    // namespace communication

    ///////////////////////////////////////////////////////////////////////////
    // support for all_to_all
    template <typename Communicator>
    struct communication_operation<Communicator, communication::all_to_all_tag>
    {
        template <typename Result, typename Payload>
        static Result get(Communicator& communicator, std::size_t which,
            std::size_t generation,
            hpx::collectives::detail::generation_mode num_generations,
            Payload&& payload)
        {
            using payload_type = std::decay_t<Payload>;
            return communicator.template handle_data<payload_type>(
                communication::communicator_data<
                    communication::all_to_all_tag>::name(),
                which, generation,
                // step function (invoked for each get)
                [&payload](auto& data, std::size_t which) {
                    data[which] = HPX_FORWARD(Payload, payload);
                },
                // finalizer (invoked after all data has been received)
                [](auto& data, auto&, std::size_t which) {
                    return finalize(data, which);
                },
                num_generations);
        }

    private:
        template <typename T>
        static std::vector<T> finalize(
            std::vector<std::vector<T>>& data, std::size_t const which)
        {
            std::vector<T> result;
            result.reserve(data.size());
            for (auto& values : data)
            {
                if (values.size() != data.size())
                {
                    HPX_THROW_EXCEPTION(hpx::error::bad_parameter,
                        "hpx::collectives::all_to_all",
                        "each participating site must contribute exactly "
                        "num_sites elements");
                }

                result.push_back(hpx::collectives::detail::handle_bool<T>(
                    HPX_MOVE(values[which])));
            }
            return result;
        }

        template <typename T>
        static hpx::collectives::detail::ragged_rows<T> finalize(
            std::vector<hpx::collectives::detail::ragged_rows<T>>& data,
            std::size_t const which)
        {
            return hpx::collectives::detail::ragged_rows<T>(data, which);
        }
    };
}    // namespace hpx::traits

namespace hpx::collectives {

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        // all_to_all: detail entry point carrying the internal generation step.
        // The public overload forwards with single_step; the hierarchical
        // overload passes double_step for the flat fast path and the inter-group
        // exchange.
        template <typename Payload>
        hpx::future<std::decay_t<Payload>> all_to_all(communicator fid,
            Payload&& local_result, this_site_arg this_site,
            generation_arg const generation, generation_mode num_generations)
        {
            using payload_type = std::decay_t<Payload>;

            if (this_site.is_default())
            {
                this_site = agas::get_locality_id();
            }
            if (generation == 0)
            {
                return hpx::make_exceptional_future<payload_type>(
                    HPX_GET_EXCEPTION(hpx::error::bad_parameter,
                        "hpx::collectives::all_to_all",
                        "the generation number shouldn't be zero"));
            }

            // Handle operation right away if there is only one value.
            if (auto [num_sites, comm_site] = fid.get_info(); num_sites == 1)
            {
                if constexpr (!is_ragged_rows_v<payload_type>)
                {
                    if (local_result.size() != 1)
                    {
                        return hpx::make_exceptional_future<payload_type>(
                            HPX_GET_EXCEPTION(hpx::error::bad_parameter,
                                "hpx::collectives::all_to_all",
                                "each participating site must contribute "
                                "exactly num_sites elements"));
                    }
                }

                if (this_site != comm_site)
                {
                    return hpx::make_exceptional_future<payload_type>(
                        HPX_GET_EXCEPTION(hpx::error::bad_parameter,
                            "hpx::collectives::all_to_all",
                            "the local site should be zero if only one site is "
                            "involved"));
                }

                if constexpr (is_ragged_rows_v<payload_type>)
                {
                    std::vector<payload_type> values;
                    values.emplace_back(HPX_FORWARD(Payload, local_result));
                    return hpx::make_ready_future(payload_type(values, 0));
                }
                else
                {
                    return hpx::make_ready_future(
                        HPX_FORWARD(Payload, local_result));
                }
            }

            auto all_to_all_data =
                [local_result = HPX_FORWARD(Payload, local_result), this_site,
                    generation, num_generations](
                    communicator&& c) mutable -> hpx::future<payload_type> {
                using action_type =
                    communicator_server::communication_get_direct_action<
                        traits::communication::all_to_all_tag,
                        hpx::future<payload_type>, generation_mode,
                        payload_type>;

                // explicitly unwrap returned future
                hpx::future<payload_type> result =
                    hpx::async(action_type(), c, this_site, generation,
                        num_generations, HPX_MOVE(local_result));

                if (!result.is_ready())
                {
                    // make sure id is kept alive as long as the returned future
                    traits::detail::get_shared_state(result)->set_on_completed(
                        [client = HPX_MOVE(c)] { HPX_UNUSED(client); });
                }

                return result;
            };

            return fid.then(hpx::launch::sync, HPX_MOVE(all_to_all_data));
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    // all_to_all
    HPX_CXX_EXPORT template <typename T>
    hpx::future<std::vector<T>> all_to_all(communicator fid,
        std::vector<T>&& local_result,
        this_site_arg this_site = this_site_arg(),
        generation_arg const generation = generation_arg())
    {
        return detail::all_to_all(HPX_MOVE(fid), HPX_MOVE(local_result),
            this_site, generation, detail::generation_mode::single_step);
    }

    HPX_CXX_EXPORT template <typename T>
    hpx::future<std::vector<T>> all_to_all(communicator fid,
        std::vector<T>&& local_result, generation_arg const generation,
        this_site_arg const this_site = this_site_arg())
    {
        return all_to_all(
            HPX_MOVE(fid), HPX_MOVE(local_result), this_site, generation);
    }

    HPX_CXX_EXPORT template <typename T>
    hpx::future<std::vector<T>> all_to_all(char const* basename,
        std::vector<T>&& local_result,
        num_sites_arg const num_sites = num_sites_arg(),
        this_site_arg const this_site = this_site_arg(),
        generation_arg const generation = generation_arg(),
        root_site_arg const root_site = root_site_arg())
    {
        return all_to_all(create_communicator(basename, num_sites, this_site,
                              generation, root_site),
            HPX_MOVE(local_result), this_site);
    }

    // Forward declaration of the hierarchical overload (defined below) so the
    // generic sync overload can resolve to it; otherwise only the flat
    // overloads above are visible at that call site.
    HPX_CXX_EXPORT template <typename T>
    hpx::future<std::vector<T>> all_to_all(
        hierarchical_communicator const& communicators,
        std::vector<T>&& local_result,
        this_site_arg this_site = this_site_arg(),
        generation_arg generation = generation_arg());

    ///////////////////////////////////////////////////////////////////////////
    // Generic sync overload: dispatches to the flat or hierarchical async
    // overload based on the communicator type passed.
    HPX_CXX_EXPORT template <typename Communicator, typename T>
        requires(is_communicator_v<std::decay_t<Communicator>>)
    std::vector<T> all_to_all(hpx::launch::sync_policy, Communicator&& fid,
        std::vector<T>&& local_result,
        this_site_arg const this_site = this_site_arg(),
        generation_arg const generation = generation_arg())
    {
        return all_to_all(HPX_FORWARD(Communicator, fid),
            HPX_MOVE(local_result), this_site, generation)
            .get();
    }

    HPX_CXX_EXPORT template <typename Communicator, typename T>
        requires(is_communicator_v<std::decay_t<Communicator>>)
    std::vector<T> all_to_all(hpx::launch::sync_policy, Communicator&& fid,
        std::vector<T>&& local_result, generation_arg const generation,
        this_site_arg const this_site = this_site_arg())
    {
        return all_to_all(HPX_FORWARD(Communicator, fid),
            HPX_MOVE(local_result), this_site, generation)
            .get();
    }

    HPX_CXX_EXPORT template <typename T>
    std::vector<T> all_to_all(hpx::launch::sync_policy, char const* basename,
        std::vector<T>&& local_result,
        num_sites_arg const num_sites = num_sites_arg(),
        this_site_arg const this_site = this_site_arg(),
        generation_arg const generation = generation_arg(),
        root_site_arg const root_site = root_site_arg())
    {
        return all_to_all(create_communicator(basename, num_sites, this_site,
                              generation, root_site),
            HPX_MOVE(local_result), this_site)
            .get();
    }
    ///////////////////////////////////////////////////////////////////////////
    // hierarchical all_to_all
    //
    // Three-phase algorithm:
    //  Phase 1 (gather):   subtree sites gather their local_result vectors
    //                      at their top-level representative.
    //  Phase 2 (exchange): representatives perform flat all_to_all at level 0,
    //                      exchanging blocks of data between groups.
    //  Phase 3 (scatter):  representatives scatter the transposed results
    //                      back to their subtree sites.
    //
    // Generation mapping (user generation k, starting from 1):
    //  - Subtree communicators: 2k-1 (gather) and 2k (scatter)
    //  - Inter-group communicator: 2k-1 (exchange), advancing the gate by two
    //    in a single step so it stays in lock-step with the subtrees
    // Each communicator therefore advances by two generations per call,
    // which lets a single instance be shared across collectives; see the
    // note on create_hierarchical_communicator.

    // Async overload (declared above; default arguments live on that
    // forward declaration).
    HPX_CXX_EXPORT template <typename T>
    hpx::future<std::vector<T>> all_to_all(
        hierarchical_communicator const& communicators,
        std::vector<T>&& local_result, this_site_arg this_site,
        generation_arg const generation)
    {
        if (generation.is_default() || generation == 0)
        {
            return hpx::make_exceptional_future<std::vector<T>>(
                HPX_GET_EXCEPTION(hpx::error::bad_parameter,
                    "hpx::collectives::all_to_all (hierarchical)",
                    "hierarchical all_to_all requires an explicit, positive "
                    "generation number for its internal generation "
                    "mapping"));
        }

        if (!detail::is_valid_hierarchical_phase_generation(generation))
        {
            return hpx::make_exceptional_future<std::vector<T>>(
                HPX_GET_EXCEPTION(hpx::error::bad_parameter,
                    "hpx::collectives::all_to_all (hierarchical)",
                    "the generation number is too large for the internal "
                    "generation mapping"));
        }

        if (this_site.is_default())
        {
            this_site = agas::get_locality_id();
        }

        if (auto const error =
                detail::validate_hierarchical_communicator(communicators,
                    this_site, "hpx::collectives::all_to_all (hierarchical)"))
        {
            return hpx::make_exceptional_future<std::vector<T>>(error);
        }

        std::size_t const num_sites_val = hpx::get<0>(communicators.get_info());
        std::size_t const arity_val = communicators.get_arity();

        // The phase-2 packing slices each gathered contribution with
        // unchecked iterator arithmetic, so a wrong-size contribution must
        // be rejected client-side before any data is sent.
        if (local_result.size() != num_sites_val)
        {
            return hpx::make_exceptional_future<std::vector<T>>(
                HPX_GET_EXCEPTION(hpx::error::bad_parameter,
                    "hpx::collectives::all_to_all (hierarchical)",
                    "each participating site must contribute exactly "
                    "num_sites elements"));
        }

        // Generation mapping: every communicator advances two generations per
        // call and must see consecutive generation numbers starting from 1 (the
        // gate blocks until all prior generations complete). There are only two
        // distinct internal generations per call -- the first (2k-1) and the
        // second (2k). The subtree communicators run their gather at the first
        // and their scatter at the second. The inter-group communicator runs
        // its single exchange at that same first generation and advances the
        // gate past the second in one step (double_step below), so it stays in
        // lock-step with the subtrees and the instance can be shared across
        // collectives. Gather, exchange and the collapsed flat fast path
        // therefore share first_gen; only the subtree scatter uses second_gen.
        auto const [first_gen, second_gen] =
            detail::hierarchical_phase_generations(generation);

        // Flat fast path: when arity >= num_sites (either because the user
        // chose a large arity or because the factory overrode arity to
        // num_sites for the flat fallback), the tree builder's leaf condition
        // (right - left < arity) fired at the root call and produced a single
        // flat communicator spanning all sites. The 3-phase algorithm then
        // collapses to a flat all_to_all; dispatch directly to avoid the
        // intermediate allocations, but still advance the gate by two (run at
        // 2k-1, double_step) so the flat instance stays shareable with other
        // collectives.
        if (arity_val >= num_sites_val)
        {
            HPX_ASSERT(communicators.size() == 1);
            return detail::all_to_all(communicators.get(0),
                HPX_MOVE(local_result), communicators.site(0), first_gen,
                detail::generation_mode::double_step);
        }

        std::ptrdiff_t const gidx =
            detail::classify_site(this_site, num_sites_val, arity_val);
        bool const is_representative =
            detail::is_top_level_rep(gidx, this_site, num_sites_val, arity_val);

        if (is_representative)
        {
            // Phase 1: Gather all subtree sites' data at this rep.
            detail::uniform_rows<T> gathered =
                detail::subtree_gather_at_top_rep(
                    communicators, HPX_MOVE(local_result), first_gen);

            std::size_t const group_index = static_cast<std::size_t>(gidx);
            std::size_t const my_group_size = detail::get_top_level_group_size(
                group_index, num_sites_val, arity_val);
            std::size_t const num_groups =
                detail::get_top_level_group_count(num_sites_val, arity_val);
            std::size_t const exchange_size = detail::checked_data_size_product(
                my_group_size, num_sites_val - my_group_size);
            std::size_t const exchange_segments =
                detail::checked_data_size_product(my_group_size, num_groups);
            std::size_t const exchange_boundaries =
                detail::checked_data_size_sum(exchange_segments, 1);
            std::size_t const diagonal_size =
                detail::checked_data_size_product(my_group_size, my_group_size);

            gathered.validate("hpx::collectives::all_to_all (hierarchical)");
            if (gathered.num_rows() != my_group_size)
            {
                HPX_THROW_EXCEPTION(hpx::error::invalid_status,
                    "hpx::collectives::all_to_all (hierarchical)",
                    "the subtree gather returned an unexpected number of "
                    "rows");
            }
            if (gathered.row_width() != num_sites_val)
            {
                HPX_THROW_EXCEPTION(hpx::error::invalid_status,
                    "hpx::collectives::all_to_all (hierarchical)",
                    "the subtree gather returned rows with an unexpected "
                    "number of elements");
            }

            // Phase 2: Pack one flat sequence of destination-group segments
            // for each gathered row. Retain the local diagonal separately and
            // represent it by empty exchange segments, avoiding its round trip
            // through the inter-group communicator.
            std::vector<T> exchange_data;
            exchange_data.reserve(exchange_size);
            std::vector<std::size_t> exchange_offsets;
            exchange_offsets.reserve(exchange_boundaries);
            exchange_offsets.push_back(0);
            std::vector<T> diagonal;
            diagonal.reserve(diagonal_size);
            for (std::size_t row = 0; row != my_group_size; ++row)
            {
                std::size_t const row_left =
                    detail::checked_data_size_product(row, num_sites_val);
                for (std::size_t group = 0; group != num_groups; ++group)
                {
                    std::size_t const dest_group_size =
                        detail::get_top_level_group_size(
                            group, num_sites_val, arity_val);
                    std::size_t const dest_left =
                        detail::get_top_level_group_left(
                            group, num_sites_val, arity_val);
                    auto& destination =
                        group == group_index ? diagonal : exchange_data;
                    for (std::size_t destination_site = dest_left;
                        destination_site != dest_left + dest_group_size;
                        ++destination_site)
                    {
                        destination.emplace_back(
                            gathered.take(row_left + destination_site));
                    }
                    exchange_offsets.push_back(exchange_data.size());
                }
            }
            HPX_ASSERT(exchange_data.size() == exchange_size);
            HPX_ASSERT(diagonal.size() == diagonal_size);

            detail::ragged_rows<T> exchange(
                HPX_MOVE(exchange_data), HPX_MOVE(exchange_offsets));

            // The exchange touches the inter-group communicator once but must
            // advance it by two generations to stay in lock-step with the
            // subtree communicators, so request double_step.
            hpx::future<detail::ragged_rows<T>> received_future =
                detail::all_to_all(communicators.get(0), HPX_MOVE(exchange),
                    communicators.site(0), first_gen,
                    detail::generation_mode::double_step);

            // Every gathered element now lives in the exchange or diagonal.
            // Reuse the gathered allocation for the final scatter rows.
            std::vector<T> scatter_data = HPX_MOVE(gathered).release_data();
            scatter_data.clear();

            detail::ragged_rows<T> received = received_future.get();
            received.validate("hpx::collectives::all_to_all (hierarchical)");
            if (received.num_segments() != num_groups)
            {
                HPX_THROW_EXCEPTION(hpx::error::invalid_status,
                    "hpx::collectives::all_to_all (hierarchical)",
                    "the inter-group exchange returned an unexpected number "
                    "of blocks");
            }
            for (std::size_t group = 0; group != num_groups; ++group)
            {
                std::size_t const source_group_size =
                    detail::get_top_level_group_size(
                        group, num_sites_val, arity_val);
                std::size_t const expected_size = group == group_index ?
                    0 :
                    detail::checked_data_size_product(
                        source_group_size, my_group_size);
                if (received.segment_size(group) != expected_size)
                {
                    HPX_THROW_EXCEPTION(hpx::error::invalid_status,
                        "hpx::collectives::all_to_all (hierarchical)",
                        "the inter-group exchange returned a block with an "
                        "unexpected number of elements");
                }
            }

            // Phase 3: Transpose the received off-diagonal blocks and the
            // retained local diagonal into one row per subtree site.
            scatter_data.reserve(detail::checked_data_size_product(
                my_group_size, num_sites_val));
            for (std::size_t j = 0; j != my_group_size; ++j)
            {
                for (std::size_t group = 0; group != num_groups; ++group)
                {
                    std::size_t const source_group_size =
                        detail::get_top_level_group_size(
                            group, num_sites_val, arity_val);
                    for (std::size_t source = 0; source != source_group_size;
                        ++source)
                    {
                        if (group == group_index)
                        {
                            std::size_t const index =
                                source * my_group_size + j;
                            scatter_data.emplace_back(detail::handle_bool<T>(
                                HPX_MOVE(diagonal[index])));
                        }
                        else
                        {
                            std::size_t const index =
                                received.offsets()[group] +
                                source * my_group_size + j;
                            scatter_data.emplace_back(received.take(index));
                        }
                    }
                }
            }
            HPX_ASSERT(scatter_data.size() ==
                detail::checked_data_size_product(
                    my_group_size, num_sites_val));

            return detail::subtree_scatter_at_top_rep(communicators,
                detail::uniform_rows<T>(HPX_MOVE(scatter_data), my_group_size),
                second_gen);
        }
        else
        {
            // Non-representative: send data to rep, then receive
            // result from rep.
            detail::subtree_send_to_top_rep(
                communicators, HPX_MOVE(local_result), first_gen);

            return detail::subtree_receive_from_top_rep<T>(
                communicators, second_gen);
        }
    }
}    // namespace hpx::collectives

////////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_ALLTOALL_DECLARATION(...) /**/

////////////////////////////////////////////////////////////////////////////////
#define HPX_REGISTER_ALLTOALL(...)             /**/

#endif    // !HPX_COMPUTE_DEVICE_CODE
#endif    // DOXYGEN
