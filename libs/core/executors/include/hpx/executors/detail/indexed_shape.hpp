//  Copyright (c) 2026 the-ivii
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/modules/memory.hpp>

#include <atomic>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>
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

        std::atomic<std::size_t> count{0};
    };

    template <typename S>
    class indexed_shape_iterator
    {
    private:
        using storage_type = indexed_shape_storage<S>;
        using position_iterator = typename std::vector<
            std::ranges::iterator_t<S const>>::const_iterator;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using iterator_concept = std::random_access_iterator_tag;
        using value_type = std::ranges::range_value_t<S const>;
        using difference_type =
            typename std::iterator_traits<position_iterator>::difference_type;
        using pointer = void;
        using reference = std::ranges::range_reference_t<S const>;

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

        reference operator*() const
        {
            return **current_;
        }

        reference operator[](difference_type n) const
        {
            return **(current_ + n);
        }

        indexed_shape_iterator& operator++()
        {
            ++current_;
            return *this;
        }

        indexed_shape_iterator operator++(int)
        {
            auto result = *this;
            ++*this;
            return result;
        }

        indexed_shape_iterator& operator--()
        {
            --current_;
            return *this;
        }

        indexed_shape_iterator operator--(int)
        {
            auto result = *this;
            --*this;
            return result;
        }

        indexed_shape_iterator& operator+=(difference_type n)
        {
            current_ += n;
            return *this;
        }

        indexed_shape_iterator& operator-=(difference_type n)
        {
            current_ -= n;
            return *this;
        }

        friend indexed_shape_iterator operator+(
            indexed_shape_iterator it, difference_type n)
        {
            it += n;
            return it;
        }

        friend indexed_shape_iterator operator+(
            difference_type n, indexed_shape_iterator it)
        {
            it += n;
            return it;
        }

        friend indexed_shape_iterator operator-(
            indexed_shape_iterator it, difference_type n)
        {
            it -= n;
            return it;
        }

        friend difference_type operator-(indexed_shape_iterator const& lhs,
            indexed_shape_iterator const& rhs)
        {
            return lhs.current_ - rhs.current_;
        }

        friend bool operator==(indexed_shape_iterator const& lhs,
            indexed_shape_iterator const& rhs)
        {
            return lhs.current_ == rhs.current_;
        }

        friend bool operator<(indexed_shape_iterator const& lhs,
            indexed_shape_iterator const& rhs)
        {
            return lhs.current_ < rhs.current_;
        }

        friend bool operator>(indexed_shape_iterator const& lhs,
            indexed_shape_iterator const& rhs)
        {
            return rhs < lhs;
        }

        friend bool operator<=(indexed_shape_iterator const& lhs,
            indexed_shape_iterator const& rhs)
        {
            return !(rhs < lhs);
        }

        friend bool operator>=(indexed_shape_iterator const& lhs,
            indexed_shape_iterator const& rhs)
        {
            return !(lhs < rhs);
        }

    private:
        hpx::intrusive_ptr<storage_type> state_;
        position_iterator current_;
    };

    // Random access shapes already provide constant-time lookup. Other
    // multipass shapes are indexed once while preserving references to their
    // elements. Only the begin iterator retains the copied shape.
    template <typename S>
        requires std::ranges::forward_range<S const>
    decltype(auto) make_indexed_shape(S const& shape)
    {
        if constexpr (std::ranges::random_access_range<S const> &&
            std::ranges::sized_range<S const>)
        {
            return (shape);
        }
        else
        {
            using storage_type = indexed_shape_storage<S>;
            using iterator = indexed_shape_iterator<S>;
            using range_type = std::ranges::subrange<iterator>;

            static_assert(std::ranges::random_access_range<range_type>);
            static_assert(std::ranges::sized_range<range_type>);

            hpx::intrusive_ptr<storage_type> state(new storage_type(shape));
            auto const first = state->positions.cbegin();
            auto const last = state->positions.cend();

            return range_type(iterator(state, first), iterator(last));
        }
    }

    /// \endcond
}    // namespace hpx::execution::experimental::detail
