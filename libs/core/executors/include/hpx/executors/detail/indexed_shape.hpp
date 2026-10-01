//  Copyright (c) 2026 the-ivii
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#include <hpx/modules/iterator_support.hpp>
#include <hpx/modules/memory.hpp>
#include <hpx/modules/thread_support.hpp>

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
          , count_(1)
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
            p->count_.increment();
        }

        friend void intrusive_ptr_release(indexed_shape_storage* p) noexcept
        {
            if (p->count_.decrement() == 0)
            {
                // The thread that decrements the reference count to zero must
                // perform an acquire to ensure that it doesn't start
                // destructing the object until all previous writes have
                // drained.
                std::atomic_thread_fence(std::memory_order_acquire);

                delete p;
            }
        }

        hpx::util::atomic_count count_;
    };

    template <typename S>
    class indexed_shape_iterator
      : public hpx::util::iterator_facade<indexed_shape_iterator<S>,
            std::iter_value_t<std::ranges::iterator_t<S const>> const,
            std::random_access_iterator_tag,
            std::iter_reference_t<std::ranges::iterator_t<S const>>>
    {
    private:
        using source_iterator = std::ranges::iterator_t<S const>;
        using position_iterator = std::vector<source_iterator>::const_iterator;

    public:
        indexed_shape_iterator() = default;

        explicit indexed_shape_iterator(position_iterator current)
          : current_(current)
        {
        }

        using use_brackets_proxy =
            hpx::util::detail::use_operator_brackets_proxy<source_iterator,
                std::iter_value_t<source_iterator> const>;

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

        position_iterator current_;
    };

    template <typename S>
    class indexed_shape
    {
    private:
        using storage_type = indexed_shape_storage<S>;

    public:
        using iterator = indexed_shape_iterator<S>;

        explicit indexed_shape(S const& shape)
          : state_(new storage_type(shape), false)
        {
        }

        iterator begin() const noexcept
        {
            return iterator(state_->positions.cbegin());
        }

        iterator end() const noexcept
        {
            return iterator(state_->positions.cend());
        }

        std::size_t size() const noexcept
        {
            return state_->positions.size();
        }

    private:
        hpx::intrusive_ptr<storage_type> state_;
    };

    // Random access shapes already provide constant-time lookup and are
    // returned by reference to avoid a preliminary copy. Executor call sites
    // copy that reference into the operation state before returning. Other
    // multipass shapes are copied and indexed once. The resulting range owns
    // that copy while its iterators remain lightweight and non-owning.
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
            using range_type = indexed_shape<S>;

            static_assert(std::ranges::random_access_range<range_type>);
            static_assert(std::ranges::sized_range<range_type>);

            return range_type(shape);
        }
    }

    /// \endcond
}    // namespace hpx::execution::experimental::detail
