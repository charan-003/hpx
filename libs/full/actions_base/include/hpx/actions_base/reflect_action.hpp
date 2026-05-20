//  Copyright (c) 2026 Priyanshi Sharma
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file reflect_action.hpp
/// \brief Reflection-based action definition for HPX remote operations.
#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_CXX26_REFLECTION)

#include <hpx/actions_base/basic_action.hpp>
#include <hpx/actions_base/plain_action.hpp>
#include <hpx/modules/serialization.hpp>

#include <cstddef>
#include <meta>
#include <string>
#include <type_traits>

namespace hpx::actions {

    /// \cond NOINTERNAL
    namespace detail {

        /// Helper to extract function type from reflection for use
        /// as basic_action template argument (two-step workaround for
        /// GCC/Clang restriction on splice expressions as template args).
        template <std::meta::info F>
        struct reflect_action_base
        {
            using func_type = [:std::meta::type_of(F):];
            using type = basic_action<hpx::actions::detail::plain_function,
                func_type, reflect_action_base<F>>;
        };

    }    // namespace detail
    /// \endcond

    /// \brief Reflection-based action template.
    ///
    /// reflect_action<F> integrates with HPX's action system by inheriting
    /// from basic_action<detail::plain_function, R(Ps...), reflect_action<F>>.
    /// All properties are derived automatically from the reflected function F.
    ///
    /// \tparam F  A std::meta::info reflection of a free function.
    template <std::meta::info F>
    struct reflect_action
      : basic_action<hpx::actions::detail::plain_function,
            typename detail::reflect_action_base<F>::func_type,
            reflect_action<F>>
    {
        /// The function type (e.g. int(double, double))
        using func_type = [:std::meta::type_of(F):];

        /// The function pointer type (e.g. int(*)(double, double))
        using func_ptr_type = std::add_pointer_t<func_type>;

        /// The actual function pointer
        static constexpr func_ptr_type func_ptr = [:F:];

        /// Compile-time storage for the fully qualified action name
        static constexpr auto name_storage =
            hpx::serialization::detail::scope_builder<F>::value;

        /// Returns the action name in HPX format: "plain action(app::compute)"
        static std::string get_action_name(
            naming::address::address_type /*lva*/)
        {
            return hpx::actions::detail::make_plain_action_name(
                std::string_view(name_storage.data, name_storage.size));
        }

        /// Invokes the reflected function with the given arguments.
        template <typename... Ts>
        static auto invoke(naming::address::address_type /*lva*/,
            naming::address::component_type /*comptype*/, Ts&&... vs)
        {
            using base_t = basic_action<hpx::actions::detail::plain_function,
                func_type, reflect_action<F>>;
            base_t::increment_invocation_count();
            return func_ptr(HPX_FORWARD(Ts, vs)...);
        }
    };

}    // namespace hpx::actions

#endif    // HPX_HAVE_CXX26_REFLECTION
