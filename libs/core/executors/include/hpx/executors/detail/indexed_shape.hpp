//  Copyright (c) 2026 the-ivii
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/modules/iterator_support.hpp>
#include <hpx/modules/memory.hpp>

#include <atomic>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <utility>
#include <vector>

namespace hpx::execution::experimental::detail {
    /// \cond NOINTERNAL

    template <typename S>
    struct indexed_shape_storage
    {
        explicit indexed_shape_storage(S const& input_shape)
          : shape(input_shape)
        {
            auto const& owned_shape = std::as_const(shape);
            if constexpr (std::ranges::sized_range<S const>)
            {
                positions.reserve(std::ranges::size(owned_shape));
            }

            for (auto it = std::ranges::begin(owned_shape);
                it != std::ranges::end(owned_shape); ++it)
            {
                positions.push_back(it);
            }
        }

        S shape;
        std::vector<std::ranges::iterator_t<S const>> positions;

    private:
        friend void intrusive_ptr_add_ref(indexed_shape_storage* p) noexcept
        {
            p->count.fetch_add(1, std::memory_order_relaxed);
        }

        friend void intrusive_ptr_release(indexed_shape_storage* p) noexcept
        {
            if (p->count.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete p;
            }
        }

        std::atomic<std::size_t> count{1};
    };

    template <typename S>
    class indexed_shape_iterator
      : public hpx::util::iterator_facade<indexed_shape_iterator<S>,
            std::iter_value_t<std::ranges::iterator_t<S const>> const,
            std::random_access_iterator_tag,
            std::iter_reference_t<std::ranges::iterator_t<S const>>>
    {
    private:
        using storage_type = indexed_shape_storage<S>;
        using position_iterator =
            std::vector<std::ranges::iterator_t<S const>>::const_iterator;

    public:
        indexed_shape_iterator() = default;

        indexed_shape_iterator(
            hpx::intrusive_ptr<storage_type> state, position_iterator current)
          : state_(HPX_MOVE(state))
          , current_(current)
        {
        }

        explicit indexed_shape_iterator(position_iterator current)
          : current_(current)
        {
        }

    private:
        friend class hpx::util::iterator_core_access;

        std::ranges::range_reference_t<S const> dereference() const
            noexcept(noexcept(**current_))
        {
            return **current_;
        }

        void increment() noexcept(noexcept(++current_))
        {
            ++current_;
        }

        void decrement() noexcept(noexcept(--current_))
        {
            --current_;
        }

        void advance(std::ptrdiff_t n) noexcept(noexcept(current_ += n))
        {
            current_ += n;
        }

        std::ptrdiff_t distance_to(indexed_shape_iterator const& other) const
            noexcept(noexcept(other.current_ - current_))
        {
            return other.current_ - current_;
        }

        bool equal(indexed_shape_iterator const& other) const
            noexcept(noexcept(current_ == other.current_))
        {
            return current_ == other.current_;
        }

        hpx::intrusive_ptr<storage_type> state_;
        position_iterator current_;
    };

    // Random access shapes already provide constant-time lookup. Other
    // multipass shapes are indexed once while preserving references to their
    // elements. Only the begin iterator retains the copied shape.
    template <typename S>
        requires(std::ranges::forward_range<S const>)
    decltype(auto) make_indexed_shape(S const& shape)
    {
        if constexpr (std::ranges::random_access_range<S const> &&
            std::ranges::sized_range<S const>)
        {
            return shape;
        }
        else
        {
            using storage_type = indexed_shape_storage<S>;
            using iterator = indexed_shape_iterator<S>;
            using range_type = std::ranges::subrange<iterator>;

            static_assert(std::ranges::random_access_range<range_type>);
            static_assert(std::ranges::sized_range<range_type>);

            hpx::intrusive_ptr<storage_type> state(
                new storage_type(shape), false);
            auto const first = state->positions.cbegin();
            auto const last = state->positions.cend();

            return range_type(iterator(state, first), iterator(last));
        }
    }

    /// \endcond
}    // namespace hpx::execution::experimental::detail
