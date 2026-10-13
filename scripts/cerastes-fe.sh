#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Run the cerastes front end (build/cerastes-fe: EDG with the cerastes
# configuration and in-process back end) the way scripts/cpfe.sh runs stock cpfe:
# emulating the installed clang and parsing the system SDK/libc++ headers.
# All arguments are passed through; --cerastes_walk is ours.  A C mode option (--c, --c99,
# --c11, --c17, --c23, ...) selects the C include path; otherwise C++.
#
# Example: scripts/cerastes-fe.sh --c++20 --cerastes_walk tests/edg-smoke/hello.cpp

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
[[ -f "$ROOT/build/env.sh" ]] || { echo "cerastes-fe.sh: run scripts/bootstrap.sh first" >&2; exit 2; }
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

exec "$CERASTES_FE" --clang_version="$EDG_CLANG_VER_SCRAPE" "${sys_includes[@]}" "$@"
