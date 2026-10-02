# Copyright (c) 2026 Rohan Pattanayak
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Windows counterpart of run_raw_gxx_smoke.sh: compile the hpx_main.hpp smoke
# with a raw cl.exe invocation (no CMake, no vcxproj), reusing the compiler
# options MSBuild already used for the CMake hello_wrap target.
#
# MSVC has no -Wl,-wrap=main. hpx/hpx_main.hpp redefines main as
# hpx_startup::user_main and hpx_wrap.lib provides the real main, so the
# link needs no extra option. The HPX module libraries are not listed on the
# link line: the installed headers name them through #pragma comment(lib),
# so only /LIBPATH to the install is needed, which tests auto-linking.
#
# Usage:
#   run_raw_msvc_smoke.ps1 -Prefix <install-prefix> `
#       -CMakeBuildDir <hello_wrap cmake build dir> [-Config Release] `
#       [-OutDir build/smoke]

param(
    [Parameter(Mandatory = $true)][string]$Prefix,
    [Parameter(Mandatory = $true)][string]$CMakeBuildDir,
    [string]$Config = 'Release',
    [string]$OutDir = 'build/smoke'
)

$ErrorActionPreference = 'Stop'

$Prefix = (Resolve-Path $Prefix).Path
$CMakeBuildDir = (Resolve-Path $CMakeBuildDir).Path
New-Item -ItemType Directory -Force $OutDir | Out-Null
$OutDir = (Resolve-Path $OutDir).Path
$Source = Join-Path $PSScriptRoot 'raw_wrap.cpp'

# Put cl.exe and link.exe on PATH.
$env:PATH = (Join-Path ${env:ProgramFiles(x86)} `
        'Microsoft Visual Studio\Installer') + ";$env:PATH"
$vs = vswhere.exe -latest -products * -property installationPath
Import-Module (Join-Path $vs `
        'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation `
    -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null

# HPX::hpx carries MSVC options (/std:, /Zc:...) and include directories
# (Boost, hwloc) a raw invocation needs as well. MSBuild records the exact cl
# command line it ran for hello_wrap in its command tlog (UTF-16); reuse it
# the way run_raw_gxx_smoke.sh reuses `ninja -t commands`.
$tlog = Join-Path $CMakeBuildDir "hello_wrap.dir\$Config\hello_wrap.tlog"

$clFlags = Get-Content -Encoding Unicode (Join-Path $tlog 'CL.command.1.tlog') |
    Where-Object { $_ -and -not $_.StartsWith('^') } |
    Select-Object -First 1

# Drop the output/PDB options and the hello_wrap source; keep everything
# else (defines, includes, /std:, /Zc:..., /EHsc, /MD, ...).
$clFlags = $clFlags `
    -replace '/c\s', '' `
    -replace '/F[od]"[^"]*"', '' `
    -replace '(?i)("[^"]+\.cpp"|\S+\.cpp)\s*$', ''

# hpx_core.lib comes from auto-linking; hpx.lib is listed explicitly as
# well. hpx_wrap.lib and hpx_init.lib are not named by any header, and
# neither are hwloc and the Windows libraries HPX::hpx adds (dbghelp.lib is
# needed for the stack traces HPX_WITH_STACKTRACES enables by default), so
# pass those explicitly.
$hwloc = Join-Path $Prefix 'hwloc_installed'
$libs = @(
    "/LIBPATH:`"$Prefix\lib`""
    "/LIBPATH:`"$hwloc\lib`""
    'hpx_wrap.lib'
    'hpx_init.lib'
    'hpx.lib'
    'libhwloc.dll.a'
    'psapi.lib'
    'shlwapi.lib'
    'dbghelp.lib'
) -join ' '

$rsp = Join-Path $OutDir 'raw_wrap.rsp'
Set-Content -Encoding ascii $rsp @(
    $clFlags
    '/DHPX_APPLICATION_EXPORTS'
    "/I`"$Prefix\include`""
    "/Fo`"$(Join-Path $OutDir 'raw_wrap.obj')`""
    "/Fe`"$(Join-Path $OutDir 'raw_wrap.exe')`""
    "`"$Source`""
    # /link and its arguments have to be on one line in a response file
    "/link $libs"
)

& cl.exe /nologo "@$rsp"
if ($LASTEXITCODE -ne 0) { throw "cl.exe failed with $LASTEXITCODE" }

# The install puts the hwloc DLL into bin.
$env:PATH = "$Prefix\bin;$env:PATH"
& "$OutDir\raw_wrap.exe" --hpx:threads=2
if ($LASTEXITCODE -ne 0) { throw "raw_wrap.exe failed with $LASTEXITCODE" }
