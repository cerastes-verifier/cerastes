#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Run the pinned EDG front end (build/edg/bin/cpfe) the way cerastes does:
# emulating the installed clang and parsing the system SDK/libc++ headers.
# All arguments are passed through to cpfe.  A C mode option (--c, --c99,
# --c11, --c17, --c23, ...) selects the C include path; otherwise C++.
#
# Example: scripts/cpfe.sh --c++20 --no_code_gen tests/edg-smoke/hello.cpp

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
[[ -f "$ROOT/build/env.sh" ]] || { echo "cpfe.sh: run scripts/bootstrap.sh first" >&2; exit 2; }
# shellcheck source=/dev/null
. "$ROOT/build/env.sh"

includes="$EDG_CLANG_INCL_SCRAPE"
for arg in "$@"; do
  if [[ "$arg" =~ ^--c([0-9]+)?$ ]]; then
    includes="$EDG_CLANG_CINCL_SCRAPE"
  fi
done

sys_includes=()
IFS=: read -r -a dirs <<<"$includes"
for dir in "${dirs[@]}"; do
  [[ -n "$dir" ]] && sys_includes+=("--sys_include=$dir")
done

exec "$CPFE" --clang_version="$EDG_CLANG_VER_SCRAPE" "${sys_includes[@]}" "$@"
