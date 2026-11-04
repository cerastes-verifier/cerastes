#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Decision 0046 item 3 (audit S5): vlibc's headers are system files for the front end in verify mode, where EDG
# drops warnings and downgrades discretionary errors in them, so this compiles every header of the active vlibc's
# include/vlibc once more as an ordinary (non-system) header, in C23 and C++20, and requires no diagnostic at all.
#
# The EDG flags are the driver's own: `cerastes --cerastes_print_edg_command <mode>` prints the arguments
# `cerastes verify` would pass, with the two vlibc directories as plain -I directories in the places where verify mode
# puts them as --sys_include (the shim first, so a bare <stdlib.h> reaches it; <vlibc>/include second, for
# <vlibc/...>).  build/cerastes-fe runs them; it defines __CERASTES__ itself, so that argument is left out.
# Each header is included twice: as the shim (`#include <name.h>`) and as `#include <vlibc/name.h>`.  Needs scripts/bootstrap.sh.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
[[ -f "$ROOT/build/env.sh" ]] || { echo "check_vlibc_headers.sh: run scripts/bootstrap.sh first" >&2; exit 2; }
# shellcheck source=/dev/null
. "$ROOT/build/env.sh"

shim="$CERASTES_VLIBC_DIR/include/vlibc"
[[ -d "$shim" ]] || { echo "check_vlibc_headers.sh: no $shim" >&2; exit 2; }
work="$(mktemp -d "${TMPDIR:-/tmp}/cerastes-vlibc-headers.XXXXXX")"
trap 'rm -rf "$work"' EXIT

failed=0
checked=0
for mode in c23 c++20; do
  args=()
  while IFS= read -r line; do
    [[ "$line" == -D__CERASTES__=* ]] || args+=("$line")
  done < <("$CERASTES_VERIFY" --cerastes_print_edg_command "$mode")
  [[ ${#args[@]} -gt 0 ]] || { echo "check_vlibc_headers.sh: no EDG arguments for $mode" >&2; exit 1; }
  # C++ only: EDG #541 (an exception specification that does not match the earlier declaration's) fires on glibc, where __THROW is
  # `noexcept(true)` and vlibc's redeclarations of `exit` and `_Exit` have none (decision 0046 item 3; GCC and Clang accept them, and cerastes
  # models no exceptions).
  [[ "$mode" == c23 ]] || args+=(--diag_suppress=541)
  ext=c
  [[ "$mode" == c23 ]] || ext=cpp
  for header in "$shim"/*.h; do
    [[ -f "$header" ]] || continue
    name="$(basename "$header")"
    for form in "$name" "vlibc/$name"; do
      tu="$work/tu.$ext"
      printf '#include <%s>\n' "$form" > "$tu"
      status=0
      "$CERASTES_FE" "${args[@]}" "$tu" > "$work/out.txt" 2>&1 || status=$?
      checked=$((checked + 1))
      if [[ $status -ne 0 || -s "$work/out.txt" ]]; then
        echo "check_vlibc_headers.sh: FAIL: #include <$form> ($mode): exit $status" >&2
        cat "$work/out.txt" >&2
        failed=$((failed + 1))
      fi
    done
  done
done
if [[ $checked -eq 0 ]]; then
  echo "check_vlibc_headers.sh: no header found in $shim" >&2
  exit 1
fi
if [[ $failed -ne 0 ]]; then
  echo "check_vlibc_headers.sh: $failed of $checked compilations had diagnostics (decision 0046 item 3)" >&2
  exit 1
fi
echo "check_vlibc_headers.sh: $checked compilations of vlibc's headers as non-system files: no diagnostics"
