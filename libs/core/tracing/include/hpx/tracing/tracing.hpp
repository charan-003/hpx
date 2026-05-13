//  Copyright (c) 2026 Hartmut Kaiser
//  Copyright (c) 2026 Vansh Dobhal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

#include <cstddef>
#include <string>

#if defined(HPX_HAVE_MODULE_TRACY)
#include <hpx/modules/tracy.hpp>

#define HPX_HAVE_TRACING 1

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct region_init_data
    {
        char const* name = nullptr;
        std::size_t thread_phase = 0;
        bool is_stackless = false;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT [[maybe_unused]] loop_context
    {
        constexpr explicit loop_context() noexcept {}

        ~loop_context() = default;

        loop_context(loop_context const&) = delete;
        loop_context& operator=(loop_context const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT region
    {
        explicit region(loop_context&, region_init_data const& init_data,
            std::size_t num_thread) noexcept;

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
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT lock_context
    {
        explicit lock_context(char const* name = nullptr) noexcept;
        explicit lock_context(std::string const& name) noexcept;

        ~lock_context();

        lock_context(lock_context const&) = delete;
        lock_context& operator=(lock_context const&) = delete;

        bool before_lock() const noexcept;
        void after_lock() const noexcept;
        void after_try_lock(bool acquired) const noexcept;
        void after_unlock() const noexcept;

    private:
        hpx::tracy::lock_data impl;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void set_thread_name(
        char const* name) noexcept;

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT char const* rename_region(
        char const* name) noexcept;

}    // namespace hpx::tracing

#elif defined(HPX_HAVE_ITTNOTIFY) && HPX_HAVE_ITTNOTIFY != 0
#include <hpx/modules/itt_notify.hpp>

#define HPX_HAVE_TRACING 1

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct region_init_data
    {
        char const* name = nullptr;
        std::size_t thread_phase = 0;
        void const* thread_ptr = nullptr;
        bool is_stackless = false;
        std::size_t address = 0;
        bool is_address_type = false;
        void* itt_string_handle = nullptr;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT loop_context
    {
        explicit loop_context() noexcept;
        ~loop_context();

        loop_context(loop_context const&) = delete;
        loop_context& operator=(loop_context const&) = delete;

        util::itt::stack_context stack_ctx;
        util::itt::thread_domain thread_domain;
        util::itt::string_handle task_id;
        util::itt::string_handle task_phase;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT region
    {
        explicit region(
            loop_context& ctx, region_init_data const& data, std::size_t);
        ~region();

        region(region const&) = delete;
        region& operator=(region const&) = delete;

    private:
        static util::itt::task make_task(
            loop_context& ctx, region_init_data const& data);

        util::itt::caller_context cctx;
        util::itt::task task;
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
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] lock_context
    {
        constexpr explicit lock_context(char const* = nullptr) noexcept {}
        constexpr explicit lock_context(std::string const&) noexcept {}

        constexpr bool before_lock() const noexcept
        {
            return false;
        }

        constexpr void after_lock() const noexcept {}

        constexpr void after_try_lock(bool) const noexcept {}

        constexpr void after_unlock() const noexcept {}
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT constexpr void set_thread_name(char const*) noexcept {}

    ////////////////////////////////////////////////////////////////////////////
    // ITT has no rename_region equivalent
    HPX_CXX_CORE_EXPORT constexpr char const* rename_region(
        char const*) noexcept
    {
        return nullptr;
    }

}    // namespace hpx::tracing

#else

#define HPX_HAVE_TRACING 0

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct region_init_data
    {
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] loop_context
    {
        constexpr explicit loop_context() noexcept {}

        ~loop_context() = default;

        loop_context(loop_context const&) = delete;
        loop_context& operator=(loop_context const&) = delete;
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] region
    {
        constexpr explicit region(
            loop_context&, region_init_data const&, std::size_t) noexcept
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
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] lock_context
    {
        constexpr explicit lock_context(char const* = nullptr) noexcept {}
        constexpr explicit lock_context(std::string const&) noexcept {}

        constexpr bool before_lock() const noexcept
        {
            return false;
        }

        constexpr void after_lock() const noexcept {}

        constexpr void after_try_lock(bool) const noexcept {}

        constexpr void after_unlock() const noexcept {}
    };

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT constexpr void set_thread_name(char const*) noexcept {}

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT constexpr char const* rename_region(
        char const*) noexcept
    {
        return nullptr;
    }

}    // namespace hpx::tracing

#endif
