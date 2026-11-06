#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# The runner setup of the macOS jobs of .github/workflows/ci.yml (decisions 0016 and
# 0053): `scripts/ci.sh <profile>`, a macOS profile of scripts/ci_toolchains.toml.
# Asserts that the machine is the toolchain the profile declares, installs what is
# missing (the profile's GCC and, through scripts/bootstrap.sh, JDK 17, sbt and the
# ShellCheck linter), builds everything and writes build/ci-toolchain.txt.
# scripts/check.sh is a separate workflow step that runs after this one.
#
# Nothing is ever upgraded or removed: the Homebrew variables below keep a rehearsal
# on a developer machine from changing it.  The toolchain is asserted at the majors
# the profile declares (decision 0053 item 1): macOS, the Command Line Tools, Apple
# clang, the SDK and GCC; the exact builds are printed and written to the toolchain
# report, not asserted, so an image update within a major does not stop the job (the
# tests decide whether a result moved).  An assertion that fails is never loosened
# here: the expected values come from the profile, which scripts/ci_profile.py
# validates, and the two references are pinned in the harness self-test as well.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

CLT=/Library/Developer/CommandLineTools
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

# ---- The profile: the expected values (decision 0053 items 1 and 4) --------------
step "profile"
[[ $# -eq 1 ]] || want "usage: scripts/ci.sh <profile>, a macOS profile of scripts/ci_toolchains.toml"
PROFILE="$1"
[[ "${CERASTES_CI_PROFILE:-$PROFILE}" == "$PROFILE" ]] || want "CERASTES_CI_PROFILE is $CERASTES_CI_PROFILE, but the profile given is $PROFILE"
PLATFORM="$(python3 -I scripts/ci_profile.py get "$PROFILE" platform)" || want "no profile $PROFILE (python3 -I scripts/ci_profile.py list)"
[[ "$PLATFORM" == macos ]] || want "$PROFILE is a $PLATFORM profile; scripts/ci.sh sets up macOS runners"
MACOS_MAJOR="$(python3 -I scripts/ci_profile.py get "$PROFILE" macos_major)" || want "the profile $PROFILE has no macos_major"
CLT_MAJOR="$(python3 -I scripts/ci_profile.py get "$PROFILE" clt_major)" || want "the profile $PROFILE has no clt_major"
CLANG_MAJOR="$(python3 -I scripts/ci_profile.py get "$PROFILE" clang_major)" || want "the profile $PROFILE has no clang_major"
SDK_MAJOR="$(python3 -I scripts/ci_profile.py get "$PROFILE" sdk_major)" || want "the profile $PROFILE has no sdk_major"
GCC_MAJOR="$(python3 -I scripts/ci_profile.py get "$PROFILE" gcc_major)" || want "the profile $PROFILE has no gcc_major"
BUILD_COMPILER="$(python3 -I scripts/ci_profile.py get "$PROFILE" build)" || want "the profile $PROFILE has no build"
[[ "$BUILD_COMPILER" == clang ]] || want "the profile $PROFILE builds with $BUILD_COMPILER; on macOS only clang builds (decision 0053 item 10)"
printf 'profile: %s (macOS %s, Command Line Tools %s, Apple clang %s, SDK %s, GCC %s)\n' "$PROFILE" "$MACOS_MAJOR" "$CLT_MAJOR" "$CLANG_MAJOR" "$SDK_MAJOR" "$GCC_MAJOR"

step "machine"
uname -a
sw_vers
df -h "$ROOT"
printf 'image:   %s %s\n' "${ImageOS:-local}" "${ImageVersion:-local}"
macos_version="$(sw_vers -productVersion)" || want "sw_vers failed"
[[ "$macos_version" =~ ^$MACOS_MAJOR(\.[0-9]+)*$ ]] || want "macOS $macos_version, expected $MACOS_MAJOR or $MACOS_MAJOR.*"
free_gb="$(df -g "$ROOT" | awk 'NR==2 {print $4}')" || want "df -g failed"
[[ "$free_gb" =~ ^[0-9]+$ ]] || want "cannot read the free disk space (df -g gave '$free_gb')"
[[ "$free_gb" -ge "$MIN_FREE_GB" ]] || want "only $free_gb GB free on $ROOT; the build and the tests need $MIN_FREE_GB GB"

step "environment"
for var in JAVA_TOOL_OPTIONS _JAVA_OPTIONS JDK_JAVA_OPTIONS JAVA_OPTS SBT_OPTS SDKROOT MACOSX_DEPLOYMENT_TARGET CC CXX CFLAGS CXXFLAGS CPPFLAGS LDFLAGS CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH OBJC_INCLUDE_PATH LIBRARY_PATH GCC_EXEC_PREFIX COMPILER_PATH CERASTES_EDG_SRC CERASTES_CLANG_MAJOR CERASTES_GCC_MAJOR CERASTES_BUILD_COMPILER; do
  [[ -z "${!var+set}" ]] || want "$var is set; the build must not depend on it (decisions 0016, 0053)"
done
printf 'CERASTES_JOBS=%s\n' "${CERASTES_JOBS:-<unset>}"

step "toolchain"
[[ "${DEVELOPER_DIR:-}" == "$CLT" ]] || want "DEVELOPER_DIR must be $CLT (is '${DEVELOPER_DIR:-}')"
clt_version="$(pkgutil --pkg-info=com.apple.pkg.CLTools_Executables | sed -n 's/^version: //p')" || want "pkgutil cannot find the Command Line Tools package"
[[ "$clt_version" == "$CLT_MAJOR".* ]] || want "Command Line Tools $clt_version, expected $CLT_MAJOR.*"
clang_path="$(command -v clang)" || want "clang is not on PATH"
[[ "$clang_path" == /usr/bin/clang ]] || want "clang on PATH is $clang_path, expected /usr/bin/clang"
# EDG's preset also builds with CXX=clang++, and cerastes's CMake with the default cc and c++.
for tool in clang++ cc c++; do
  [[ "$(command -v "$tool")" == "/usr/bin/$tool" ]] || want "$tool on PATH is '$(command -v "$tool")', expected /usr/bin/$tool"
done
xcrun_clang="$(xcrun -f clang)" || want "xcrun cannot find clang"
[[ "$xcrun_clang" == "$CLT/usr/bin/clang" ]] || want "xcrun selects $xcrun_clang, expected $CLT/usr/bin/clang"
clang_banner="$(clang --version | sed -n 1p)" || want "clang --version failed"
[[ "$clang_banner" == "Apple clang version $CLANG_MAJOR."* ]] || want "clang is '$clang_banner', expected Apple clang $CLANG_MAJOR"
sdk_version="$(xcrun --show-sdk-version)" || want "xcrun cannot report the SDK version"
[[ "$sdk_version" =~ ^$SDK_MAJOR(\.[0-9]+)*$ ]] || want "SDK $sdk_version, expected $SDK_MAJOR or $SDK_MAJOR.*"
sdk_build="$(xcrun --show-sdk-build-version)" || want "xcrun cannot report the SDK build version"
[[ -n "$sdk_build" ]] || want "xcrun reports an empty SDK build version"
sdk_path="$(xcrun --show-sdk-path)" || want "xcrun cannot report the SDK path"
sdk_real="$(realpath "$sdk_path")" || want "cannot resolve the SDK path $sdk_path"
[[ "$sdk_real" == "$CLT/SDKs/MacOSX$sdk_version.sdk" ]] || want "SDK is $sdk_real, expected $CLT/SDKs/MacOSX$sdk_version.sdk"
printf 'clt:     %s\nclang:   %s\nsdk:     %s %s (%s)\n' "$clt_version" "$clang_banner" "$sdk_version" "$sdk_build" "$sdk_real"
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
command -v "g++-$GCC_MAJOR" >/dev/null || env -u HOMEBREW_NO_INSTALL_UPGRADE brew install "gcc@$GCC_MAJOR"
gxx="$(command -v "g++-$GCC_MAJOR")" || want "g++-$GCC_MAJOR not found"
gcc_cc="$(command -v "gcc-$GCC_MAJOR")" || want "gcc-$GCC_MAJOR not found"
gcc_version="$("$gxx" -dumpfullversion)" || want "$gxx -dumpfullversion failed"
[[ "$gcc_version" == "$GCC_MAJOR".* ]] || want "g++ is $gcc_version, expected $GCC_MAJOR.*"
gcc_sysroot="$("$gcc_cc" -print-sysroot)" || want "$gcc_cc -print-sysroot failed"
# The native-ok runs compile against GCC's own sysroot: it must be the SDK asserted above
# (a macOS 26 bottle names MacOSX26.sdk, which the Command Line Tools 27 also ship).
gcc_sysroot_real="$(realpath "$gcc_sysroot")" || want "the sysroot '$gcc_sysroot' of $gcc_cc does not exist"
[[ "$gcc_sysroot_real" == "$sdk_real" ]] || want "the sysroot of $gcc_cc is $gcc_sysroot_real, expected the SDK $sdk_real"
printf 'g++:     %s %s\ngcc:     %s\nsysroot: %s\n' "$gxx" "$gcc_version" "$gcc_cc" "$gcc_sysroot"

step "bootstrap"
# the profile's compilers (decision 0053 item 6): bootstrap records them for check.sh's --build-only
export CERASTES_CLANG_MAJOR="$CLANG_MAJOR"
export CERASTES_GCC_MAJOR="$GCC_MAJOR"
export CERASTES_BUILD_COMPILER="$BUILD_COMPILER"
scripts/bootstrap.sh

step "build/env.sh"
# shellcheck source=/dev/null
. build/env.sh
[[ "$CERASTES_CLANG_CXX" == "$CLT/usr/bin/clang++" ]] || want "build/env.sh has CERASTES_CLANG_CXX=$CERASTES_CLANG_CXX, expected $CLT/usr/bin/clang++"
[[ "$CERASTES_GCC_CXX" == "$gxx" ]] || want "build/env.sh has CERASTES_GCC_CXX=$CERASTES_GCC_CXX, expected $gxx"
[[ "$CERASTES_BUILD_COMPILER" == "$BUILD_COMPILER" ]] || want "build/env.sh has CERASTES_BUILD_COMPILER=$CERASTES_BUILD_COMPILER, expected $BUILD_COMPILER"
[[ "$CERASTES_BUILD_CXX" == /usr/bin/clang++ ]] || want "build/env.sh has CERASTES_BUILD_CXX=$CERASTES_BUILD_CXX, expected /usr/bin/clang++ (EDG's preset)"
[[ "$EDG_CLANG_VER_SCRAPE" =~ ^[1-9][0-9]{4,}$ ]] || want "build/env.sh has EDG_CLANG_VER_SCRAPE=$EDG_CLANG_VER_SCRAPE, not a clang version"
[[ "$((EDG_CLANG_VER_SCRAPE / 10000))" == "$CLANG_MAJOR" ]] || want "build/env.sh has EDG_CLANG_VER_SCRAPE=$EDG_CLANG_VER_SCRAPE, expected clang $CLANG_MAJOR (${CLANG_MAJOR}0000 to ${CLANG_MAJOR}9999)"

step "toolchain report"
{
  printf 'profile: %s\n' "$PROFILE"
  printf 'image:   %s %s\n' "${ImageOS:-local}" "${ImageVersion:-local}"
  printf 'macos:   %s\n' "$macos_version"
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
