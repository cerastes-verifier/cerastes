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
# The tree the caller asks for (decision 0005) must be the one build/env.sh was
# written for; read it before sourcing env.sh.
requested="${CERASTES_EDG_SRC:-}"
if [[ -n "$requested" && "$requested" != /* ]]; then
  echo "cpfe.sh: CERASTES_EDG_SRC=$requested is a relative path; use an absolute path (decision 0005)" >&2
  exit 2
fi
[[ -n "$requested" ]] || requested="$ROOT/third_party/edg"
requested_real="$(cd "$requested" 2>/dev/null && pwd -P || printf '%s' "$requested")"
# shellcheck source=/dev/null
. "$ROOT/build/env.sh"
if [[ "$requested_real" != "$CERASTES_EDG_SRC_ACTIVE" ]]; then
  echo "cpfe.sh: build/env.sh is for $CERASTES_EDG_KIND $CERASTES_EDG_SRC_ACTIVE but CERASTES_EDG_SRC selects $requested_real; run scripts/bootstrap.sh" >&2
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

exec "$CPFE" --clang_version="$EDG_CLANG_VER_SCRAPE" "${sys_includes[@]}" "$@"
