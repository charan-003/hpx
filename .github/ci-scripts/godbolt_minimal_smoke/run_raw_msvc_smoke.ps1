# Copyright (c) 2026 Rohan Pattanayak
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Windows counterpart of run_raw_gxx_smoke.sh: compile the hpx_main.hpp smoke
# with a raw cl.exe invocation (no CMake, no vcxproj), reusing the compiler
# options and libraries MSBuild already used for the CMake hello_wrap target.
#
# MSVC has no -Wl,-wrap=main. hpx/hpx_main.hpp redefines main as
# hpx_startup::user_main and hpx_wrap.lib provides the real main, so the
# link needs no extra option.
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

# Compiler Explorer invokes the compiler directly. HPX static builds install
# one .lib per module, which are not merged into hpx_core.lib, and HPX::hpx
# carries MSVC options (/std:, /Zc:...) a raw invocation needs as well.
# MSBuild records the exact cl and link command lines it ran for hello_wrap
# in its command tlogs (UTF-16); reuse them the way run_raw_gxx_smoke.sh
# reuses `ninja -t commands`.
$tlog = Join-Path $CMakeBuildDir "hello_wrap.dir\$Config\hello_wrap.tlog"

function Get-CommandLine([string]$file)
{
    Get-Content -Encoding Unicode (Join-Path $tlog $file) |
        Where-Object { $_ -and -not $_.StartsWith('^') } |
        Select-Object -First 1
}

# Drop the output/PDB options and the hello_wrap source; keep everything
# else (defines, includes, /std:, /Zc:..., /EHsc, /MD, ...).
$clFlags = (Get-CommandLine 'CL.command.1.tlog') `
    -replace '/c\s', '' `
    -replace '/F[od]"[^"]*"', '' `
    -replace '(?i)("[^"]+\.cpp"|\S+\.cpp)\s*$', ''

# Keep only the libraries (.lib, and .dll.a for the fetched hwloc). Relative
# ones are relative to the CMake build directory, so run cl from there.
$libs = ([regex]::Matches((Get-CommandLine 'link.command.1.tlog'),
        '(?i)"[^"]+\.(lib|a)"|\S+\.(lib|a)(?=\s|$)') |
    ForEach-Object { $_.Value } |
    Where-Object { $_ -notmatch '(?i)hello_wrap\.lib' }) -join ' '

$rsp = Join-Path $OutDir 'raw_wrap.rsp'
Set-Content -Encoding ascii $rsp @(
    $clFlags
    '/DHPX_APPLICATION_EXPORTS'
    "/I`"$Prefix\include`""
    "/Fo`"$OutDir\\`""
    "/Fe`"$OutDir\raw_wrap.exe`""
    "`"$Source`""
    # /link and its arguments have to be on one line in a response file
    "/link $libs"
)

Push-Location $CMakeBuildDir
try
{
    & cl.exe /nologo "@$rsp"
    if ($LASTEXITCODE -ne 0) { throw "cl.exe failed with $LASTEXITCODE" }
}
finally
{
    Pop-Location
}

# run_cmake_smoke.sh --copy-dlls already put the hwloc DLL next to the CMake
# smoke binaries.
$env:PATH = "$CMakeBuildDir\$Config;$env:PATH"
& "$OutDir\raw_wrap.exe" --hpx:threads=2
if ($LASTEXITCODE -ne 0) { throw "raw_wrap.exe failed with $LASTEXITCODE" }
