//  Copyright (c) 2026 Priyanshi Sharma
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file reflect_action.hpp
/// \brief Reflection-based action definition for HPX remote operations.
///
/// This header provides reflect_action<F>, a C++26 reflection-based
/// replacement for the HPX_PLAIN_ACTION and HPX_REGISTER_ACTION macros.
/// Instead of verbose boilerplate, users write a single line:
///
///   using compute_action = HPX_ACTION(app::compute);
///
/// The action name, function pointer type, arity, and registration are
/// all derived automatically at compile time using C++26 static reflection.

#pragma once

#include <hpx/config.hpp>

#if defined(HPX_HAVE_CXX26_REFLECTION)

#include <hpx/serialization/detail/refl_qualified_name_of.hpp>

#include <cstddef>
#include <meta>

namespace hpx::actions {

    /// \brief Reflection-based action template.
    ///
    /// reflect_action<F> provides the same interface as HPX_PLAIN_ACTION
    /// but derives all properties automatically from the reflected function F:
    ///   - Qualified name via scope_builder<F>
    ///   - Function pointer type via std::meta::type_of(F)
    ///   - Function pointer via splicing [:F:]
    ///   - Arity via std::meta::parameters_of(F).size()
    ///
    /// \tparam F  A std::meta::info reflection of a free function.
    ///            Obtain via the reflection operator: ^^app::my_function
    template <std::meta::info F>
    struct reflect_action
    {
        /// The function pointer type (e.g. int(*)(double, double))
        // Note: std::add_pointer_t<[:std::meta::type_of(F):]> is not yet supported
        // by current Clang P2996 as a template argument. Using splice pointer instead.
        using func_ptr_type = [:std::meta::type_of(F):]*;

        /// The actual function pointer
        static constexpr func_ptr_type func_ptr = [:F:];

        /// Compile-time storage for the fully qualified action name
        static constexpr auto name_storage =
            hpx::serialization::detail::scope_builder<F>::value;

        /// Number of parameters the function takes
        static constexpr std::size_t arity = std::meta::parameters_of(F).size();

        /// Returns the fully qualified name of the action.
        /// Called by hpx::actions::detail::register_action during
        /// static initialization to register this action with the
        /// HPX action registry.
        static consteval char const* get_action_name() noexcept
        {
            return name_storage.data;
        }
    };

}    // namespace hpx::actions

#endif    // HPX_HAVE_CXX26_REFLECTION
