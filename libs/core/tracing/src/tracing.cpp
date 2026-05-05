//  Copyright (c) 2026 Hartmut Kaiser
//  Copyright (c) 2026 Vansh Dobhal
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/tracing/tracing.hpp>

#include <cstddef>
#include <memory>

#if defined(HPX_HAVE_MODULE_TRACY)

namespace hpx::tracing {

    ////////////////////////////////////////////////////////////////////////////
    // region

    hpx::tracy::region region::create_tracy_region(
        region_init_data const& data, std::size_t const num_thread) noexcept
    {
        bool const enabled = data.name != nullptr && !data.is_stackless;
        return hpx::tracy::region(
            data.name, num_thread, data.thread_phase, enabled);
    }

    region::region(
        region_init_data const& data, std::size_t const num_thread) noexcept
      : impl(create_tracy_region(data, num_thread))
    {
    }

    region::~region() = default;

    ////////////////////////////////////////////////////////////////////////////
    // mark_event

    mark_event::mark_event(char const* name) noexcept
      : impl(name)
    {
    }

    mark_event::~mark_event() = default;

    ////////////////////////////////////////////////////////////////////////////
    // fiber_region

    hpx::tracy::fiber_region fiber_region::create_tracy_fiber_region(
        fiber_region_init_data const& data,
        std::size_t const num_thread) noexcept
    {
        bool const enabled = data.name != nullptr && !data.is_stackless;
        char const* fiber_name = enabled ? data.fiber_name : nullptr;

        // Use num_thread as color seed so each worker thread gets a distinct
        // color on the fiber track in Tracy.
        auto const color =
            static_cast<std::size_t>(num_thread + 1) * 0x9e3779b9;
        return hpx::tracy::fiber_region(fiber_name, data.name, color, enabled);
    }

    fiber_region::fiber_region(fiber_region_init_data const& data,
        std::size_t const num_thread) noexcept
      : impl(create_tracy_fiber_region(data, num_thread))
    {
    }

    fiber_region::~fiber_region() = default;

    ////////////////////////////////////////////////////////////////////////////
    // fiber_suspend_region

    fiber_suspend_region::fiber_suspend_region(char const* desc) noexcept
      : impl(desc)
    {
    }

    fiber_suspend_region::~fiber_suspend_region() = default;

    ////////////////////////////////////////////////////////////////////////////
    // set_thread_name

    void set_thread_name(char const* name) noexcept
    {
        hpx::tracy::set_thread_name(name);
    }

}    // namespace hpx::tracing

#endif

#if !defined(HPX_HAVE_MODULE_TRACY) && defined(HPX_HAVE_ITTNOTIFY) &&          \
    HPX_HAVE_ITTNOTIFY != 0 && !defined(HPX_HAVE_APEX)
#include <hpx/modules/itt_notify.hpp>

namespace hpx::tracing {

    struct itt_loop_context::impl
    {
        util::itt::stack_context stack_ctx;
        util::itt::thread_domain thread_domain;
        util::itt::string_handle task_id;
        util::itt::string_handle task_phase;

        impl()
          : task_id("task_id")
          , task_phase("task_phase")
        {
        }
    };

    itt_loop_context::itt_loop_context()
      : impl_(std::make_unique<impl>())
    {
    }

    itt_loop_context::~itt_loop_context() = default;

    struct itt_task_region::impl
    {
        util::itt::caller_context cctx;
        util::itt::task task;

        static util::itt::task make_task(
            itt_loop_context::impl& ctx, thread_region_init_data const& data)
        {
            if (data.is_address_type)
            {
                return util::itt::task(ctx.thread_domain,
                    util::itt::string_handle("address"), data.address);
            }
            if (data.itt_string_handle != nullptr)
            {
                return util::itt::task(ctx.thread_domain,
                    util::itt::string_handle(static_cast<___itt_string_handle*>(
                        data.itt_string_handle)));
            }
            return util::itt::task(
                ctx.thread_domain, util::itt::string_handle(data.name));
        }

        impl(itt_loop_context::impl& ctx, thread_region_init_data const& data)
          : cctx(ctx.stack_ctx, !data.is_stackless)
          , task(make_task(ctx, data))
        {
            task.add_metadata(ctx.task_id, data.thread_ptr);
            task.add_metadata(ctx.task_phase, data.thread_phase);
        }
    };

    itt_task_region::itt_task_region(
        itt_loop_context& ctx, thread_region_init_data const& data)
      : impl_(std::make_unique<impl>(*ctx.impl_, data))
    {
    }

    itt_task_region::~itt_task_region() = default;

}    // namespace hpx::tracing

#endif
