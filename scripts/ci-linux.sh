#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# The runner setup of the Linux job of .github/workflows/ci.yml (decision 0031), the
# counterpart of scripts/ci.sh: asserts that the machine is GitHub's ubuntu-26.04 x86-64
# image with the toolchain the Linux expectations were rehearsed on, installs what is
# missing from Ubuntu's archive (GCC 16, libc++ 21; never an upgrade of what the image
# has), builds everything and writes build/ci-toolchain.txt.  scripts/check.sh is a
# separate workflow step that runs after this one.
#
# The compilers are asserted at their majors (decision 0053 item 1) and glibc exactly;
# the package versions and banners are printed and written to the toolchain report, not
# asserted.  An assertion that fails is never loosened here; the expected values are the
# constants below, recorded in decisions 0001, 0031 and 0053, and they change together
# with those records.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# ---- Expected values (decisions 0001, 0031 and 0053: change them together) -----
OS_ID=ubuntu
OS_VERSION=26.04
ARCH=x86_64
GLIBC_VERSION=2.43
CLANG_MAJOR=21
GCC_MAJOR=16
MIN_FREE_GB=6

export DEBIAN_FRONTEND=noninteractive

step() { printf '\n==== %s\n' "$*"; }
want() { printf 'ci-linux.sh: %s\n' "$*" >&2; exit 1; }
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
# The installed version of a Debian package, or nothing.
pkg_version() { dpkg-query -W -f='${Status} ${Version}\n' "$1" 2>/dev/null | sed -n 's/^install ok installed //p'; }

step "machine"
uname -a
cat /etc/os-release
df -h "$ROOT"
printf 'image:   %s %s\n' "${ImageOS:-local}" "${ImageVersion:-local}"
# shellcheck source=/dev/null
os_id="$(. /etc/os-release && printf '%s' "$ID")"
# shellcheck source=/dev/null
os_version="$(. /etc/os-release && printf '%s' "$VERSION_ID")"
[[ "$os_id" == "$OS_ID" && "$os_version" == "$OS_VERSION" ]] || want "the OS is $os_id $os_version, expected $OS_ID $OS_VERSION"
[[ "$(uname -m)" == "$ARCH" ]] || want "the machine is $(uname -m), expected $ARCH"
free_gb="$(df -BG --output=avail "$ROOT" | sed -n '2s/[^0-9]//gp')" || want "df failed"
[[ "$free_gb" =~ ^[0-9]+$ ]] || want "cannot read the free disk space (df gave '$free_gb')"
[[ "$free_gb" -ge "$MIN_FREE_GB" ]] || want "only $free_gb GB free on $ROOT; the build and the tests need $MIN_FREE_GB GB"

step "environment"
for var in JAVA_TOOL_OPTIONS _JAVA_OPTIONS JDK_JAVA_OPTIONS JAVA_OPTS SBT_OPTS CC CXX CFLAGS CXXFLAGS CPPFLAGS LDFLAGS CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH LIBRARY_PATH LD_LIBRARY_PATH LD_PRELOAD GCC_EXEC_PREFIX COMPILER_PATH CERASTES_EDG_SRC; do
  [[ -z "${!var+set}" ]] || want "$var is set; the build must not depend on it (decisions 0016, 0031)"
done
printf 'CERASTES_JOBS=%s\n' "${CERASTES_JOBS:-<unset>}"
[[ -n "${JAVA_HOME_17_X64:-}" ]] || want "JAVA_HOME_17_X64 is not set; the image's JDK 17 is the one bootstrap uses"

step "packages"
# Ubuntu's archive as the image's apt sources name it; installs only what is missing.
sudo apt-get update -q
sudo apt-get install -y -q --no-install-recommends "gcc-$GCC_MAJOR" "g++-$GCC_MAJOR" \
  "clang-$CLANG_MAJOR" "libc++-$CLANG_MAJOR-dev" "libc++abi-$CLANG_MAJOR-dev" shellcheck
# One LLVM build for the compiler and the library headers, one GCC build for both drivers.
llvm_package_version="$(pkg_version "clang-$CLANG_MAJOR")"
[[ -n "$llvm_package_version" ]] || want "clang-$CLANG_MAJOR is not installed"
for pkg in "libc++-$CLANG_MAJOR-dev" "libc++abi-$CLANG_MAJOR-dev"; do
  got="$(pkg_version "$pkg")"
  [[ "$got" == "$llvm_package_version" ]] || want "$pkg is '${got:-<not installed>}', expected $llvm_package_version (the version of clang-$CLANG_MAJOR)"
done
gcc_package_version="$(pkg_version "gcc-$GCC_MAJOR")"
[[ -n "$gcc_package_version" ]] || want "gcc-$GCC_MAJOR is not installed"
got="$(pkg_version "g++-$GCC_MAJOR")"
[[ "$got" == "$gcc_package_version" ]] || want "g++-$GCC_MAJOR is '${got:-<not installed>}', expected $gcc_package_version (the version of gcc-$GCC_MAJOR)"

step "toolchain"
glibc="$(ldd --version | sed -n '1s/.* //p')" || want "ldd --version failed"
[[ "$glibc" == "$GLIBC_VERSION" ]] || want "glibc is $glibc, expected $GLIBC_VERSION (the headers EDG parses)"
clang_cc="$(command -v "clang-$CLANG_MAJOR")" || want "clang-$CLANG_MAJOR is not on PATH"
clang_cxx="$(command -v "clang++-$CLANG_MAJOR")" || want "clang++-$CLANG_MAJOR is not on PATH"
[[ "$clang_cc" == "/usr/bin/clang-$CLANG_MAJOR" && "$clang_cxx" == "/usr/bin/clang++-$CLANG_MAJOR" ]] \
  || want "clang-$CLANG_MAJOR is $clang_cc and clang++-$CLANG_MAJOR $clang_cxx, expected the packages' /usr/bin ones"
clang_banner="$("$clang_cxx" --version | sed -n 1p)" || want "$clang_cxx --version failed"
[[ "$clang_banner" == *"clang version $CLANG_MAJOR."* ]] || want "$clang_cxx is '$clang_banner', expected clang $CLANG_MAJOR"
banner="$("$clang_cc" --version | sed -n 1p)" || want "$clang_cc --version failed"
[[ "$banner" == "$clang_banner" ]] || want "$clang_cc is '$banner', expected the banner of $clang_cxx, '$clang_banner'"
gxx="$(command -v "g++-$GCC_MAJOR")" || want "g++-$GCC_MAJOR not found"
gcc_cc="$(command -v "gcc-$GCC_MAJOR")" || want "gcc-$GCC_MAJOR not found"
gcc_version="$("$gxx" -dumpfullversion)" || want "$gxx -dumpfullversion failed"
[[ "$gcc_version" == "$GCC_MAJOR".* ]] || want "$gxx is $gcc_version, expected $GCC_MAJOR.*"
got="$("$gcc_cc" -dumpfullversion)" || want "$gcc_cc -dumpfullversion failed"
[[ "$got" == "$gcc_version" ]] || want "$gcc_cc is $got, expected $gcc_version (the version of $gxx)"
# bootstrap's native-ok search takes the highest gcc-NN on PATH: it must be this one.
gcc_highest="$(compgen -c gcc- | sed -n 's/^gcc-\([0-9][0-9]*\)$/\1/p' | sort -n | tail -1)" || want "cannot list the gcc-NN on PATH"
[[ "$gcc_highest" == "$GCC_MAJOR" ]] || want "the highest gcc-NN on PATH is gcc-$gcc_highest, expected gcc-$GCC_MAJOR (the native-ok runs use the highest)"
printf 'clang:   %s %s (%s)\ng++:     %s %s (%s)\nglibc:   %s\n' "$clang_cxx" "$clang_banner" "$llvm_package_version" "$gxx" "$gcc_version" "$gcc_package_version" "$glibc"
ver python3 --version
ver bash --version
ver git --version
ver cmake --version
ver ninja --version
ver node --version
ver npm --version
ver shellcheck --version

step "edg tree"
edg_shallow="$(git -C third_party/edg rev-parse --is-shallow-repository)" || want "third_party/edg is not an initialized submodule"
[[ "$edg_shallow" == false ]] || want "third_party/edg is a shallow clone; the checkout needs fetch-depth: 0"
git -C third_party/edg rev-parse -q --verify refs/remotes/origin/cerastes >/dev/null || want "third_party/edg has no origin/cerastes; the submodule clone must keep all remote branches"
python3 scripts/edg_tree.py check

step "bootstrap"
scripts/bootstrap.sh

step "build/env.sh"
# shellcheck source=/dev/null
. build/env.sh
[[ "$CERASTES_CLANG_CXX" == "$clang_cxx" ]] || want "build/env.sh has CERASTES_CLANG_CXX=$CERASTES_CLANG_CXX, expected $clang_cxx"
[[ "$CERASTES_GCC_CXX" == "$gxx" ]] || want "build/env.sh has CERASTES_GCC_CXX=$CERASTES_GCC_CXX, expected $gxx"
[[ "$EDG_CLANG_VER_SCRAPE" =~ ^[1-9][0-9]{4,}$ ]] || want "build/env.sh has EDG_CLANG_VER_SCRAPE=$EDG_CLANG_VER_SCRAPE, not a clang version"
[[ "$((EDG_CLANG_VER_SCRAPE / 10000))" == "$CLANG_MAJOR" ]] || want "build/env.sh has EDG_CLANG_VER_SCRAPE=$EDG_CLANG_VER_SCRAPE, expected clang $CLANG_MAJOR (${CLANG_MAJOR}0000 to ${CLANG_MAJOR}9999)"
[[ ":$EDG_CLANG_INCL_SCRAPE:" == *":/usr/lib/llvm-$CLANG_MAJOR/include/c++/v1:"* ]] \
  || want "the C++ include directories do not name libc++ $CLANG_MAJOR: $EDG_CLANG_INCL_SCRAPE"
[[ "$EDG_CLANG_INCL_SCRAPE" != *"/c++/$GCC_MAJOR"* && "$EDG_CLANG_INCL_SCRAPE" != *"include/c++/1"[0-9]* ]] \
  || want "the C++ include directories name libstdc++: $EDG_CLANG_INCL_SCRAPE"

step "toolchain report"
{
  printf 'image:   %s %s\n' "${ImageOS:-local}" "${ImageVersion:-local}"
  printf 'os:      %s %s %s\n' "$os_id" "$os_version" "$(uname -m)"
  printf 'glibc:   %s (libc6-dev %s, linux-libc-dev %s)\n' "$glibc" "$(pkg_version libc6-dev)" "$(pkg_version linux-libc-dev)"
  printf 'clang:   %s (%s)\n' "$clang_banner" "$llvm_package_version"
  printf 'gcc:     %s %s (%s)\n' "$gxx" "$gcc_version" "$gcc_package_version"
  printf 'cxx inc: %s\n' "$EDG_CLANG_INCL_SCRAPE"
  printf 'c inc:   %s\n' "$EDG_CLANG_CINCL_SCRAPE"
  printf 'java:    %s\n' "$("$CERASTES_JAVA" -version 2>&1 | sed -n 2p)"
  printf 'z3:      %s\n' "$("$Z3_EXE" --version)"
  ver python3 --version
  ver bash --version
  ver cmake --version
  ver ninja --version
  ver node --version
  printf 'jobs:    %s\n' "${CERASTES_JOBS:-<unset>}"
} | tee build/ci-toolchain.txt
df -h "$ROOT"
