// Copyright (c) 2026 The STE||AR Group
// Copyright (c) 2026 Vansh Dobhal
//
// SPDX-License-Identifier: BSL-1.0
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/init.hpp>
#include <hpx/modules/itt_notify.hpp>

int hpx_main(int, char*[])
{
    int sync_obj = 0;

    HPX_ITT_THREAD_SET_NAME("hpx_itt_smoke");

    auto* domain = HPX_ITT_DOMAIN_CREATE("hpx.itt.smoke");
    auto* task_name = HPX_ITT_STRING_HANDLE_CREATE("smoke_task");

    HPX_ITT_SYNC_CREATE(&sync_obj, "smoke_sync", "smoke_sync");
    HPX_ITT_SYNC_PREPARE(&sync_obj);

    HPX_ITT_TASK_BEGIN(domain, task_name);
    HPX_ITT_TASK_END(domain);

    HPX_ITT_SYNC_ACQUIRED(&sync_obj);
    HPX_ITT_SYNC_RELEASED(&sync_obj);
    HPX_ITT_SYNC_DESTROY(&sync_obj);

    int mark = 0;
    HPX_ITT_MARK_CREATE(mark, "smoke_mark");
    HPX_ITT_MARK(mark, "smoke_event");
    HPX_ITT_MARK_OFF(mark);

    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::local::init(&hpx_main, argc, argv);
}
