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
# The tree the caller asks for (decision 0005) must be the one build/env.sh was
# written for; read it before sourcing env.sh.
requested="${CERASTES_EDG_SRC:-}"
if [[ -n "$requested" && "$requested" != /* ]]; then
  echo "cerastes-fe.sh: CERASTES_EDG_SRC=$requested is a relative path; use an absolute path (decision 0005)" >&2
  exit 2
fi
[[ -n "$requested" ]] || requested="$ROOT/third_party/edg"
requested_real="$(cd "$requested" 2>/dev/null && pwd -P || printf '%s' "$requested")"
# shellcheck source=/dev/null
. "$ROOT/build/env.sh"
if [[ "$requested_real" != "$CERASTES_EDG_SRC_ACTIVE" ]]; then
  echo "cerastes-fe.sh: build/env.sh is for $CERASTES_EDG_KIND $CERASTES_EDG_SRC_ACTIVE but CERASTES_EDG_SRC selects $requested_real; run scripts/bootstrap.sh" >&2
  exit 2
fi

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

# include/ holds <cerastes.h> (decision 0008).  As a system include directory,
# diagnostics for macros used in the caller's file still point at the caller's
# file, and the header's own warnings are not repeated in every test.
sys_includes+=("--sys_include=$CERASTES_INCLUDE_DIR")

exec "$CERASTES_FE" --clang_version="$EDG_CLANG_VER_SCRAPE" "${sys_includes[@]}" "$@"
