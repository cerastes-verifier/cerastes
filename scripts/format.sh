#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# The code style of cerastes's tool code (decision 0051): applies, or with --check only checks, the style of
# src/.clang-format with the pinned clang-format (scripts/bootstrap.sh downloads PyPI's 23.1.3 wheel into
# build/tools; build/env.sh names it in CERASTES_CLANG_FORMAT).
#
# The files are the tracked ones of src/ (.cpp, .h, .inc) and include/cerastes.h, less the files that are not
# C++ source we write: src/driver/edg_options.inc is generated (scripts/check_edg_options.sh compares it byte
# for byte), src/backend/diagnostics.def is an X-macro table that scripts/run_tests.py reads with a regular
# expression, and src/driver/tool_paths.h.in is a CMake template.  The style is src/.clang-format and
# include/.clang-format (byte-identical, compared here); the root .clang-format says DisableFormat, so nothing else
# is ever formatted, tests/, doc/ and third_party/ included (invariant 1).
#
# Usage: scripts/format.sh [--check]
#   (no option)  format the files in place.
#   --check      change nothing; name each file that differs and exit 1 if there is one.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WANT=23.1.3                     # the pin in scripts/bootstrap.sh (CLANG_FORMAT_VERSION)

check=0
for arg in "$@"; do
  case "$arg" in
    --check) check=1 ;;
    -h|--help) sed -n '5,18p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "format.sh: unknown option: $arg" >&2; exit 2 ;;
  esac
done

if [[ -z "${CERASTES_CLANG_FORMAT:-}" ]]; then
  [[ -f "$ROOT/build/env.sh" ]] || { echo "format.sh: run scripts/bootstrap.sh first" >&2; exit 2; }
  # shellcheck source=/dev/null
  . "$ROOT/build/env.sh"
fi
CF="${CERASTES_CLANG_FORMAT:-}"
[[ -x "$CF" ]] || { echo "format.sh: no pinned clang-format; run scripts/bootstrap.sh" >&2; exit 2; }
"$CF" --version | grep -q "clang-format version $WANT\$" \
  || { echo "format.sh: $CF is not clang-format $WANT; run scripts/bootstrap.sh" >&2; exit 2; }

cd "$ROOT"
# The style is src/.clang-format and include/.clang-format, byte-identical; the root .clang-format disables
# formatting everywhere else (fail-closed: tests, doc examples, fixtures and the submodules are never touched).
for style in src/.clang-format include/.clang-format; do
  [[ -f "$style" ]] || { echo "format.sh: $style is missing (the style lives there, decision 0051)" >&2; exit 2; }
done
cmp -s src/.clang-format include/.clang-format \
  || { echo "format.sh: src/.clang-format and include/.clang-format differ; they must be byte-identical" >&2; exit 2; }
files=()
while IFS= read -r f; do
  case "$f" in
    src/driver/edg_options.inc|src/backend/diagnostics.def|src/driver/tool_paths.h.in) continue ;;
  esac
  files+=("$f")
done < <(git ls-files -- 'src/*.cpp' 'src/*.h' 'src/*.inc' include/cerastes.h)
[[ ${#files[@]} -gt 0 ]] || { echo "format.sh: git lists no source files" >&2; exit 2; }

if [[ $check -eq 0 ]]; then
  "$CF" -i --style=file "${files[@]}"
  echo "format.sh: formatted ${#files[@]} files"
  exit 0
fi

# --dry-run reports each violation as "file:line:col: error: ..."; the file names are what we want, once each.
set +e
out="$("$CF" --dry-run --Werror --style=file "${files[@]}" 2>&1)"
rc=$?
set -e
if [[ $rc -ne 0 ]]; then
  bad="$(sed -n 's/^\(.*\):[0-9][0-9]*:[0-9][0-9]*: error: .*$/\1/p' <<<"$out" | sort -u)"
  if [[ -z "$bad" ]]; then
    printf '%s\n' "$out" >&2          # not a style violation (a bad option, an unreadable file)
    exit 2
  fi
  echo "format.sh: these files differ from the style (run scripts/format.sh):" >&2
  while IFS= read -r f; do printf '  %s\n' "$f" >&2; done <<<"$bad"
  exit 1
fi
echo "format.sh: ${#files[@]} files conform"
