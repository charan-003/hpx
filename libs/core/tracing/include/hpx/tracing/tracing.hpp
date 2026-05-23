//  Copyright (c) 2026 Hartmut Kaiser
//  Copyright (c) 2026 Vansh Dobhal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace hpx::threads {

    struct thread_description;
    struct thread_id;

}    // namespace hpx::threads

namespace hpx::util::external_timer {

    struct task_wrapper;

}    // namespace hpx::util::external_timer

#if defined(HPX_HAVE_TRACY)
#include <hpx/modules/tracy.hpp>

#include <hpx/config/warnings_prefix.hpp>

namespace hpx::tracing {

    HPX_CXX_CORE_EXPORT using enable_parent_task_handler_type = bool (*)();

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
    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT [[maybe_unused]] lock_context
    {
        explicit lock_context(char const* name = nullptr) noexcept;
        explicit lock_context(char const* prefix, char const* suffix) noexcept;

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

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] task_timer_data
    {
    };

    HPX_CXX_CORE_EXPORT constexpr task_timer_data create_task_timer(
        threads::thread_description const&, std::uint32_t,
        threads::thread_id const&) noexcept
    {
        return {};
    }

    HPX_CXX_CORE_EXPORT constexpr void update_task_timer(
        task_timer_data&, char const*) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] scoped_task_timer
    {
        constexpr explicit scoped_task_timer(task_timer_data) noexcept {}

        constexpr void stop() noexcept {}
        constexpr void yield() noexcept {}
    };

    HPX_CXX_CORE_EXPORT constexpr void tracing_init(
        char const*, int, char**, std::uint32_t = 0, std::uint32_t = 1) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void tracing_finalize() noexcept {}

    HPX_CXX_CORE_EXPORT constexpr void register_thread(char const*) noexcept {}

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void create_counter(
        std::string const& name) noexcept;

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void sample_counter(
        std::string const& name, std::string const& short_name,
        double value) noexcept;

    HPX_CXX_CORE_EXPORT constexpr void send_parcel(
        std::uint64_t, std::uint64_t, std::uint64_t) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void recv_parcel(
        std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void set_enable_parent_task_handler(
        enable_parent_task_handler_type) noexcept
    {
    }

}    // namespace hpx::tracing

#include <hpx/config/warnings_suffix.hpp>

#elif defined(HPX_HAVE_ITTNOTIFY) && HPX_HAVE_ITTNOTIFY != 0
#include <hpx/modules/itt_notify.hpp>

namespace hpx::tracing {

    HPX_CXX_CORE_EXPORT using enable_parent_task_handler_type = bool (*)();

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
        constexpr explicit lock_context(char const*, char const*) noexcept {}

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

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] task_timer_data
    {
    };

    HPX_CXX_CORE_EXPORT constexpr task_timer_data create_task_timer(
        threads::thread_description const&, std::uint32_t,
        threads::thread_id const&) noexcept
    {
        return {};
    }

    HPX_CXX_CORE_EXPORT constexpr void update_task_timer(
        task_timer_data&, char const*) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] scoped_task_timer
    {
        constexpr explicit scoped_task_timer(task_timer_data) noexcept {}

        constexpr void stop() noexcept {}
        constexpr void yield() noexcept {}
    };

    HPX_CXX_CORE_EXPORT constexpr void tracing_init(
        char const*, int, char**, std::uint32_t = 0, std::uint32_t = 1) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void tracing_finalize() noexcept {}

    HPX_CXX_CORE_EXPORT constexpr void register_thread(char const*) noexcept {}

    HPX_CXX_CORE_EXPORT constexpr void create_counter(
        std::string const&) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void sample_counter(
        std::string const&, std::string const&, double) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void send_parcel(
        std::uint64_t, std::uint64_t, std::uint64_t) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void recv_parcel(
        std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void set_enable_parent_task_handler(
        enable_parent_task_handler_type) noexcept
    {
    }

}    // namespace hpx::tracing

#elif defined(HPX_HAVE_APEX)

#include <memory>

namespace hpx::tracing {

    HPX_CXX_CORE_EXPORT using enable_parent_task_handler_type = bool (*)();

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
        constexpr explicit lock_context(char const*, char const*) noexcept {}

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

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct task_timer_data
    {
        std::shared_ptr<util::external_timer::task_wrapper> data;

        task_timer_data() noexcept = default;

        explicit task_timer_data(
            std::shared_ptr<util::external_timer::task_wrapper> data) noexcept
          : data(HPX_MOVE(data))
        {
        }

        operator std::shared_ptr<util::external_timer::task_wrapper>()
            const noexcept
        {
            return data;
        }
    };

    namespace detail {

        HPX_CXX_CORE_EXPORT struct task_timer_data_access
        {
            static std::shared_ptr<util::external_timer::task_wrapper> get(
                task_timer_data const& timer_data) noexcept
            {
                return timer_data.data;
            }
        };

    }    // namespace detail

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT task_timer_data create_task_timer(
        threads::thread_description const& description,
        std::uint32_t parent_locality_id,
        threads::thread_id const& parent_task);

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void update_task_timer(
        task_timer_data& timer, char const* new_name);

    HPX_CXX_CORE_EXPORT struct HPX_CORE_EXPORT scoped_task_timer
    {
        explicit scoped_task_timer(task_timer_data data) noexcept;
        ~scoped_task_timer();

        scoped_task_timer(scoped_task_timer const&) = delete;
        scoped_task_timer& operator=(scoped_task_timer const&) = delete;

        void stop() noexcept;
        void yield() noexcept;

    private:
        bool stopped_;
        task_timer_data data_;
    };

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void tracing_init(char const* name,
        int argc, char** argv, std::uint32_t rank = 0, std::uint32_t size = 1);

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void tracing_finalize();

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void register_thread(char const* name);

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void create_counter(
        std::string const& name) noexcept;

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void sample_counter(
        std::string const& name, std::string const& short_name,
        double value) noexcept;

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void send_parcel(std::uint64_t tag,
        std::uint64_t size, std::uint64_t target_locality_id) noexcept;

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void recv_parcel(std::uint64_t tag,
        std::uint64_t size, std::uint64_t source_locality_id,
        std::uint64_t source_thread_id) noexcept;

    HPX_CXX_CORE_EXPORT HPX_CORE_EXPORT void set_enable_parent_task_handler(
        enable_parent_task_handler_type f);

}    // namespace hpx::tracing

#else

namespace hpx::tracing {

    HPX_CXX_CORE_EXPORT using enable_parent_task_handler_type = bool (*)();

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
        constexpr explicit lock_context(char const*, char const*) noexcept {}

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

    ////////////////////////////////////////////////////////////////////////////
    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] task_timer_data
    {
    };

    HPX_CXX_CORE_EXPORT constexpr task_timer_data create_task_timer(
        threads::thread_description const&, std::uint32_t,
        threads::thread_id const&) noexcept
    {
        return {};
    }

    HPX_CXX_CORE_EXPORT constexpr void update_task_timer(
        task_timer_data&, char const*) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT struct [[maybe_unused]] scoped_task_timer
    {
        constexpr explicit scoped_task_timer(task_timer_data) noexcept {}

        constexpr void stop() noexcept {}
        constexpr void yield() noexcept {}
    };

    HPX_CXX_CORE_EXPORT constexpr void tracing_init(
        char const*, int, char**, std::uint32_t = 0, std::uint32_t = 1) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void tracing_finalize() noexcept {}

    HPX_CXX_CORE_EXPORT constexpr void register_thread(char const*) noexcept {}

    HPX_CXX_CORE_EXPORT constexpr void create_counter(
        std::string const&) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void sample_counter(
        std::string const&, std::string const&, double) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void send_parcel(
        std::uint64_t, std::uint64_t, std::uint64_t) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void recv_parcel(
        std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t) noexcept
    {
    }

    HPX_CXX_CORE_EXPORT constexpr void set_enable_parent_task_handler(
        enable_parent_task_handler_type) noexcept
    {
    }

}    // namespace hpx::tracing

#endif
