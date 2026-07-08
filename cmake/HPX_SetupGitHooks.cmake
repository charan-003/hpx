# Copyright (c) 2026 Sai Charan Arvapally
# Copyright (c) 2026 The STE||AR Group
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(HPX_SETUP_GIT_HOOKS_LOADED TRUE)

function(hpx_setup_git_hooks)
  if(NOT EXISTS "${HPX_SOURCE_DIR}/.git")
    return()
  endif()

  if(NOT EXISTS "${HPX_SOURCE_DIR}/.githooks/pre-commit")
    hpx_warn("Git hooks script not found at .githooks/pre-commit")
    return()
  endif()

  find_package(Git QUIET)
  if(NOT GIT_FOUND)
    hpx_warn("Git not found; skipping git hook setup")
    return()
  endif()

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" config core.hooksPath .githooks
    WORKING_DIRECTORY "${HPX_SOURCE_DIR}"
    RESULT_VARIABLE _hpx_git_hooks_result
    ERROR_VARIABLE _hpx_git_hooks_error
  )

  if(_hpx_git_hooks_result EQUAL 0)
    hpx_info("Git commit hooks enabled (.githooks/)")
  else()
    hpx_warn(
      "Could not enable git commit hooks: ${_hpx_git_hooks_error}"
    )
  endif()
endfunction()
