//  Copyright (c) 2026 Anshuman Agrawal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file detail/flattened_data.hpp

#pragma once

#include <hpx/config.hpp>

#include <hpx/assert.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/serialization.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace hpx::collectives::detail {

    // A compact carrier for equally sized logical rows. Gather and scatter
    // preserve a uniform row width, so a row count is sufficient to derive
    // every boundary without allocating or serializing an offset table.
    HPX_CXX_EXPORT template <typename T>
    struct uniform_rows
    {
        std::vector<T> data;
        std::size_t num_rows = 0;

        template <typename Archive>
        void serialize(Archive& ar, unsigned int const)
        {
            ar & data & num_rows;
        }
    };

    template <typename T>
    struct is_uniform_rows : std::false_type
    {
    };

    template <typename T>
    struct is_uniform_rows<uniform_rows<T>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool is_uniform_rows_v =
        is_uniform_rows<std::decay_t<T>>::value;

    // Distinguishes the internal row scatter protocol from an ordinary scatter
    // whose user value type happens to be uniform_rows<T>.
    struct uniform_scatter_tag
    {
    };

    HPX_CXX_EXPORT [[nodiscard]] inline std::size_t checked_data_size_sum(
        std::size_t const lhs, std::size_t const rhs)
    {
        constexpr std::size_t max_size =
            (std::numeric_limits<std::size_t>::max)();
        if (rhs > max_size - lhs)
        {
            HPX_THROW_EXCEPTION(hpx::error::bad_parameter,
                "hpx::collectives::detail::checked_data_size_sum",
                "the hierarchical collective payload size "
                "overflows size_t");
        }
        return lhs + rhs;
    }

    HPX_CXX_EXPORT [[nodiscard]] inline std::size_t checked_data_size_product(
        std::size_t const lhs, std::size_t const rhs)
    {
        constexpr std::size_t max_size =
            (std::numeric_limits<std::size_t>::max)();
        if (rhs != 0 && lhs > max_size / rhs)
        {
            HPX_THROW_EXCEPTION(hpx::error::bad_parameter,
                "hpx::collectives::detail::checked_data_size_product",
                "the hierarchical collective payload size "
                "overflows size_t");
        }
        return lhs * rhs;
    }

    HPX_CXX_EXPORT template <typename T>
    [[nodiscard]] bool is_valid_uniform_rows(
        uniform_rows<T> const& value) noexcept
    {
        return value.num_rows == 0 ? value.data.empty() :
                                     value.data.size() % value.num_rows == 0;
    }

    HPX_CXX_EXPORT template <typename T>
    void validate_uniform_rows(
        uniform_rows<T> const& value, char const* const operation)
    {
        if (!is_valid_uniform_rows(value))
        {
            HPX_THROW_EXCEPTION(hpx::error::bad_parameter, operation,
                "the uniform-row hierarchical collective payload has an "
                "invalid row count");
        }
    }

    template <typename T>
    [[nodiscard]] std::size_t uniform_row_width(uniform_rows<T> const& value)
    {
        HPX_ASSERT(is_valid_uniform_rows(value));
        return value.num_rows == 0 ? 0 : value.data.size() / value.num_rows;
    }

    template <typename T, typename Iterator>
    void append_data_range(
        std::vector<T>& destination, Iterator first, Iterator const last)
    {
        // Inserting a range can instantiate vector's move-assignment path even
        // at end(). Append element-wise so the internal carriers require only
        // move construction. vector<bool> additionally needs proxy conversion.
        for (; first != last; ++first)
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                destination.push_back(static_cast<bool>(*first));
            }
            else
            {
                destination.emplace_back(HPX_MOVE(*first));
            }
        }
    }

    template <typename T>
    [[nodiscard]] uniform_rows<T> make_uniform_rows(std::vector<T>&& values)
    {
        uniform_rows<T> result;
        result.data = HPX_MOVE(values);
        result.num_rows = result.data.size();
        return result;
    }

    template <typename T>
    [[nodiscard]] uniform_rows<T> make_uniform_row(std::vector<T>&& values)
    {
        uniform_rows<T> result;
        result.data = HPX_MOVE(values);
        result.num_rows = 1;
        return result;
    }

    template <typename T>
    [[nodiscard]] uniform_rows<std::decay_t<T>> make_uniform_value(T&& value)
    {
        uniform_rows<std::decay_t<T>> result;
        result.data.emplace_back(HPX_FORWARD(T, value));
        result.num_rows = 1;
        return result;
    }

    HPX_CXX_EXPORT template <typename T>
    [[nodiscard]] uniform_rows<T> merge_uniform_rows(
        std::vector<uniform_rows<T>>&& values)
    {
        std::size_t total_size = 0;
        std::size_t total_rows = 0;
        std::size_t row_width = 0;
        bool have_row_width = false;

        for (auto const& value : values)
        {
            validate_uniform_rows(
                value, "hpx::collectives::detail::merge_uniform_rows");
            total_size = checked_data_size_sum(total_size, value.data.size());
            total_rows = checked_data_size_sum(total_rows, value.num_rows);

            if (value.num_rows != 0)
            {
                std::size_t const current_width = uniform_row_width(value);
                if (have_row_width && current_width != row_width)
                {
                    HPX_THROW_EXCEPTION(hpx::error::bad_parameter,
                        "hpx::collectives::detail::merge_uniform_rows",
                        "all uniform hierarchical collective rows must "
                        "have the same width");
                }
                row_width = current_width;
                have_row_width = true;
            }
        }

        uniform_rows<T> result;
        result.data.reserve(total_size);
        result.num_rows = total_rows;
        for (auto& value : values)
        {
            append_data_range(
                result.data, value.data.begin(), value.data.end());
        }

        HPX_ASSERT(is_valid_uniform_rows(result));
        return result;
    }

    HPX_CXX_EXPORT template <typename T>
    [[nodiscard]] uniform_rows<T> extract_uniform_rows(uniform_rows<T>& value,
        std::size_t const slice, std::size_t const num_slices)
    {
        // Communicator finalizers invoke this under the server lock. Each site
        // consumes a disjoint row range from value.
        HPX_ASSERT(is_valid_uniform_rows(value));
        HPX_ASSERT(num_slices != 0 && slice < num_slices);

        std::size_t const division_steps = value.num_rows / num_slices;
        std::size_t const remainder = value.num_rows % num_slices;
        std::size_t const first_row =
            slice * division_steps + (std::min) (slice, remainder);
        std::size_t const row_count =
            division_steps + (slice < remainder ? 1 : 0);
        std::size_t const row_width = uniform_row_width(value);
        std::size_t const first =
            checked_data_size_product(first_row, row_width);
        std::size_t const count =
            checked_data_size_product(row_count, row_width);
        std::size_t const last = checked_data_size_sum(first, count);

        uniform_rows<T> result;
        result.data.reserve(count);
        result.num_rows = row_count;
        append_data_range(
            result.data, value.data.begin() + first, value.data.begin() + last);

        HPX_ASSERT(is_valid_uniform_rows(result));
        return result;
    }

    template <typename T>
    [[nodiscard]] T unwrap_uniform_value(uniform_rows<T>&& value)
    {
        HPX_ASSERT(is_valid_uniform_rows(value));
        HPX_ASSERT(value.num_rows == 1 && value.data.size() == 1);

        if constexpr (std::is_same_v<T, bool>)
        {
            return static_cast<bool>(value.data.front());
        }
        else
        {
            return HPX_MOVE(value.data.front());
        }
    }

    template <typename T>
    [[nodiscard]] std::vector<T> unwrap_uniform_row(uniform_rows<T>&& value)
    {
        HPX_ASSERT(is_valid_uniform_rows(value));
        HPX_ASSERT(value.num_rows == 1);
        return HPX_MOVE(value.data);
    }

}    // namespace hpx::collectives::detail
