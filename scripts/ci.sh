#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# The runner setup of .github/workflows/ci.yml (decision 0016): asserts that the
# machine is the toolchain the expectations were made on, installs what is
# missing (GCC 16 and, through scripts/bootstrap.sh, JDK 17, sbt and shellcheck),
# builds everything and writes build/ci-toolchain.txt.  scripts/check.sh is a
# separate workflow step that runs after this one.
#
# Nothing is ever upgraded or removed: the Homebrew variables below keep a rehearsal
# on a developer machine from changing it.  An assertion that fails is never loosened
# here; the expected values are the constants below, recorded in decisions 0001 and
# 0016, and they change together with those records.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# ---- Expected values (decisions 0001 and 0016: change them together) -----------
CLT=/Library/Developer/CommandLineTools
CLT_VERSION=27.0.0.0.1788430756
CLANG_BANNER='Apple clang version 21.0.0 (clang-2100.3.34.2)'
SDK_VERSION=27.0
SDK_BUILD=26A425
GCC_MAJOR=16
GCC_VERSION=16.2.0
EDG_CLANG_VER=210000
MIN_FREE_GB=6

export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_INSTALL_UPGRADE=1
export HOMEBREW_NO_INSTALL_CLEANUP=1
export HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1

step() { printf '\n==== %s\n' "$*"; }
want() { printf 'ci.sh: %s\n' "$*" >&2; exit 1; }
# Print "tool: first line of its version output", or say it is missing.
ver() {
  local tool="$1"
  shift
  if command -v "$tool" >/dev/null; then
    printf '%-8s %s\n' "$tool" "$("$tool" "$@" 2>&1 | sed -n 1p)"
  else
    printf '%-8s <missing>\n' "$tool"
  fi
}

step "machine"
uname -a
sw_vers
df -h "$ROOT"
printf 'image:   %s %s\n' "${ImageOS:-local}" "${ImageVersion:-local}"
free_gb="$(df -g "$ROOT" | awk 'NR==2 {print $4}')" || want "df -g failed"
[[ "$free_gb" =~ ^[0-9]+$ ]] || want "cannot read the free disk space (df -g gave '$free_gb')"
[[ "$free_gb" -ge "$MIN_FREE_GB" ]] || want "only $free_gb GB free on $ROOT; the build and the tests need $MIN_FREE_GB GB"

step "environment"
for var in JAVA_TOOL_OPTIONS _JAVA_OPTIONS JDK_JAVA_OPTIONS JAVA_OPTS SBT_OPTS SDKROOT MACOSX_DEPLOYMENT_TARGET CC CXX CFLAGS CXXFLAGS CPPFLAGS LDFLAGS CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH LIBRARY_PATH GCC_EXEC_PREFIX COMPILER_PATH CERASTES_EDG_SRC; do
  [[ -z "${!var+set}" ]] || want "$var is set; the build must not depend on it (decision 0016)"
done
printf 'CERASTES_JOBS=%s\n' "${CERASTES_JOBS:-<unset>}"

step "toolchain"
[[ "${DEVELOPER_DIR:-}" == "$CLT" ]] || want "DEVELOPER_DIR must be $CLT (is '${DEVELOPER_DIR:-}')"
clt_version="$(pkgutil --pkg-info=com.apple.pkg.CLTools_Executables | sed -n 's/^version: //p')" || want "pkgutil cannot find the Command Line Tools package"
[[ "$clt_version" == "$CLT_VERSION" ]] || want "Command Line Tools $clt_version, expected $CLT_VERSION"
clang_path="$(command -v clang)" || want "clang is not on PATH"
[[ "$clang_path" == /usr/bin/clang ]] || want "clang on PATH is $clang_path, expected /usr/bin/clang"
# EDG's preset also builds with CXX=clang++, and cerastes's CMake with the default cc and c++.
for tool in clang++ cc c++; do
  [[ "$(command -v "$tool")" == "/usr/bin/$tool" ]] || want "$tool on PATH is '$(command -v "$tool")', expected /usr/bin/$tool"
done
xcrun_clang="$(xcrun -f clang)" || want "xcrun cannot find clang"
[[ "$xcrun_clang" == "$CLT/usr/bin/clang" ]] || want "xcrun selects $xcrun_clang, expected $CLT/usr/bin/clang"
clang_banner="$(clang --version | sed -n 1p)" || want "clang --version failed"
[[ "$clang_banner" == "$CLANG_BANNER" ]] || want "clang is '$clang_banner', expected '$CLANG_BANNER'"
sdk_version="$(xcrun --show-sdk-version)" || want "xcrun cannot report the SDK version"
[[ "$sdk_version" == "$SDK_VERSION" ]] || want "SDK $sdk_version, expected $SDK_VERSION"
sdk_build="$(xcrun --show-sdk-build-version)" || want "xcrun cannot report the SDK build version"
[[ "$sdk_build" == "$SDK_BUILD" ]] || want "SDK build $sdk_build, expected $SDK_BUILD (the headers EDG parses)"
sdk_path="$(xcrun --show-sdk-path)" || want "xcrun cannot report the SDK path"
sdk_real="$(realpath "$sdk_path")" || want "cannot resolve the SDK path $sdk_path"
[[ "$sdk_real" == "$CLT/SDKs/MacOSX$SDK_VERSION.sdk" ]] || want "SDK is $sdk_real, expected $CLT/SDKs/MacOSX$SDK_VERSION.sdk"
printf 'clang:   %s\nsdk:     %s %s (%s)\n' "$clang_banner" "$sdk_version" "$sdk_build" "$sdk_real"
ver python3 --version
ver bash --version
ver git --version
ver cmake --version
ver ninja --version
ver node --version
ver npm --version

step "edg tree"
edg_shallow="$(git -C third_party/edg rev-parse --is-shallow-repository)" || want "third_party/edg is not an initialized submodule"
[[ "$edg_shallow" == false ]] || want "third_party/edg is a shallow clone; the checkout needs fetch-depth: 0"
git -C third_party/edg rev-parse -q --verify refs/remotes/origin/cerastes >/dev/null || want "third_party/edg has no origin/cerastes; the submodule clone must keep all remote branches"
python3 scripts/edg_tree.py check

step "gcc"
command -v "g++-$GCC_MAJOR" >/dev/null || env -u HOMEBREW_NO_INSTALL_UPGRADE brew install gcc
gxx="$(command -v "g++-$GCC_MAJOR")" || want "g++-$GCC_MAJOR not found"
gcc_cc="$(command -v "gcc-$GCC_MAJOR")" || want "gcc-$GCC_MAJOR not found"
gcc_version="$("$gxx" -dumpfullversion)" || want "$gxx -dumpfullversion failed"
[[ "$gcc_version" == "$GCC_VERSION" ]] || want "g++ is $gcc_version, expected $GCC_VERSION"
gcc_sysroot="$("$gcc_cc" -print-sysroot)" || want "$gcc_cc -print-sysroot failed"
# The native-ok runs compile against GCC's own sysroot: it must be the SDK asserted above
# (a macOS 26 bottle names MacOSX26.sdk, which the Command Line Tools 27 also ship).
gcc_sysroot_real="$(realpath "$gcc_sysroot")" || want "the sysroot '$gcc_sysroot' of $gcc_cc does not exist"
[[ "$gcc_sysroot_real" == "$sdk_real" ]] || want "the sysroot of $gcc_cc is $gcc_sysroot_real, expected the SDK $sdk_real"
printf 'g++:     %s %s\ngcc:     %s\nsysroot: %s\n' "$gxx" "$gcc_version" "$gcc_cc" "$gcc_sysroot"

step "bootstrap"
scripts/bootstrap.sh

step "build/env.sh"
# shellcheck source=/dev/null
. build/env.sh
[[ "$CERASTES_CLANG_CXX" == "$CLT/usr/bin/clang++" ]] || want "build/env.sh has CERASTES_CLANG_CXX=$CERASTES_CLANG_CXX, expected $CLT/usr/bin/clang++"
[[ "$CERASTES_GCC_CXX" == "$gxx" ]] || want "build/env.sh has CERASTES_GCC_CXX=$CERASTES_GCC_CXX, expected $gxx"
[[ "$EDG_CLANG_VER_SCRAPE" == "$EDG_CLANG_VER" ]] || want "build/env.sh has EDG_CLANG_VER_SCRAPE=$EDG_CLANG_VER_SCRAPE, expected $EDG_CLANG_VER"

step "toolchain report"
{
  printf 'image:   %s %s\n' "${ImageOS:-local}" "${ImageVersion:-local}"
  printf 'clt:     %s\n' "$clt_version"
  printf 'clang:   %s\n' "$clang_banner"
  printf 'sdk:     %s %s (%s)\n' "$sdk_version" "$sdk_build" "$sdk_real"
  printf 'gcc:     %s %s (sysroot %s)\n' "$gxx" "$gcc_version" "$gcc_sysroot"
  printf 'java:    %s\n' "$("$CERASTES_JAVA" -version 2>&1 | sed -n 1p)"
  printf 'z3:      %s\n' "$("$Z3_EXE" --version)"
  ver python3 --version
  ver bash --version
  ver cmake --version
  ver ninja --version
  ver node --version
  printf 'jobs:    %s\n' "${CERASTES_JOBS:-<unset>}"
} | tee build/ci-toolchain.txt
df -h "$ROOT"
