//  Copyright (c) 2026 Hartmut Kaiser
//  Copyright (c) 2026 Vansh Dobhal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

#include <cstddef>
#include <memory>

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct thread_region_init_data
    {
        char const* name = nullptr;
        std::size_t thread_phase = 0;
        void const* thread_ptr = nullptr;
        bool is_stackless = false;
        std::size_t address = 0;
        bool is_address_type = false;
        void* itt_string_handle = nullptr;
    };

}    // namespace hpx::tracing

#if defined(HPX_HAVE_MODULE_TRACY)
#include <hpx/modules/tracy.hpp>

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct region_init_data
    {
        char const* name = nullptr;
        std::size_t thread_phase = 0;
        bool is_stackless = false;
    };

    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT region
    {
        explicit region(
            region_init_data const& init_data, std::size_t num_thread) noexcept;

        ~region();

    private:
        static hpx::tracy::region create_tracy_region(
            region_init_data const& data, std::size_t num_thread) noexcept;

        hpx::tracy::region impl;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT mark_event
    {
        explicit mark_event(char const* name) noexcept;
        ~mark_event();

    private:
        hpx::tracy::mark_event impl;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct fiber_region_init_data
    {
        char const* name = nullptr;
        char const* fiber_name = nullptr;
        bool is_stackless = false;
    };

    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT fiber_region
    {
        explicit fiber_region(fiber_region_init_data const& data,
            std::size_t num_thread) noexcept;

        ~fiber_region();

    private:
        static hpx::tracy::fiber_region create_tracy_fiber_region(
            fiber_region_init_data const& data,
            std::size_t num_thread) noexcept;

        hpx::tracy::fiber_region impl;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT fiber_suspend_region
    {
        explicit fiber_suspend_region(char const* desc) noexcept;
        ~fiber_suspend_region();

    private:
        hpx::tracy::fiber_suspend_region impl;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT [[maybe_unused]] itt_loop_context
    {
        constexpr explicit itt_loop_context() noexcept {}

        ~itt_loop_context() = default;

        itt_loop_context(itt_loop_context const&) = delete;
        itt_loop_context& operator=(itt_loop_context const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT [[maybe_unused]] itt_task_region
    {
        constexpr explicit itt_task_region(
            itt_loop_context&, thread_region_init_data const&) noexcept
        {
        }

        ~itt_task_region() = default;

        itt_task_region(itt_task_region const&) = delete;
        itt_task_region& operator=(itt_task_region const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void set_thread_name(
        char const* name) noexcept;

}    // namespace hpx::tracing

#elif defined(HPX_HAVE_ITTNOTIFY) && HPX_HAVE_ITTNOTIFY != 0 &&                \
    !defined(HPX_HAVE_APEX)

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct region_init_data
    {
    };

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] region
    {
        constexpr explicit region(region_init_data const&, std::size_t) noexcept
        {
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] mark_event
    {
        constexpr explicit mark_event(char const*) noexcept {}
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct fiber_region_init_data
    {
    };

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] fiber_region
    {
        constexpr explicit fiber_region(
            fiber_region_init_data const&, std::size_t) noexcept
        {
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] fiber_suspend_region
    {
        constexpr explicit fiber_suspend_region(char const*) noexcept {}
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT itt_loop_context
    {
        explicit itt_loop_context();
        ~itt_loop_context();

        itt_loop_context(itt_loop_context const&) = delete;
        itt_loop_context& operator=(itt_loop_context const&) = delete;

    private:
        struct impl;
        std::unique_ptr<impl> impl_;

        friend struct itt_task_region;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT itt_task_region
    {
        explicit itt_task_region(
            itt_loop_context& ctx, thread_region_init_data const& data);
        ~itt_task_region();

        itt_task_region(itt_task_region const&) = delete;
        itt_task_region& operator=(itt_task_region const&) = delete;

    private:
        struct impl;
        std::unique_ptr<impl> impl_;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT constexpr void set_thread_name(char const*) noexcept {}

}    // namespace hpx::tracing

#else

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct region_init_data
    {
    };

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] region
    {
        constexpr explicit region(region_init_data const&, std::size_t) noexcept
        {
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] mark_event
    {
        constexpr explicit mark_event(char const*) noexcept {}
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct fiber_region_init_data
    {
    };

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] fiber_region
    {
        constexpr explicit fiber_region(
            fiber_region_init_data const&, std::size_t) noexcept
        {
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] fiber_suspend_region
    {
        constexpr explicit fiber_suspend_region(char const*) noexcept {}
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] itt_loop_context
    {
        constexpr explicit itt_loop_context() noexcept {}

        ~itt_loop_context() = default;

        itt_loop_context(itt_loop_context const&) = delete;
        itt_loop_context& operator=(itt_loop_context const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] itt_task_region
    {
        constexpr explicit itt_task_region(
            itt_loop_context&, thread_region_init_data const&) noexcept
        {
        }

        ~itt_task_region() = default;

        itt_task_region(itt_task_region const&) = delete;
        itt_task_region& operator=(itt_task_region const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT constexpr void set_thread_name(char const*) noexcept {}

}    // namespace hpx::tracing

#endif
