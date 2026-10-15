#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Fail unless src/driver/edg_options.inc lists exactly the long options that the
# active EDG tree's cmd_line.c describes.  The driver resolves abbreviations
# against that list to refuse options (decision 0006, 0007); a stale list lets a
# newly added EDG option, or one removed from EDG, be mis-resolved.  Regenerate
# with `scripts/check_edg_options.sh --update`.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
[[ -f "$ROOT/build/env.sh" ]] || { echo "check_edg_options.sh: run scripts/bootstrap.sh first" >&2; exit 2; }
# shellcheck source=/dev/null
. "$ROOT/build/env.sh"
SRC="${CERASTES_EDG_SRC_ACTIVE:?build/env.sh does not set CERASTES_EDG_SRC_ACTIVE}/src/cmd_line.c"
INC="$ROOT/src/driver/edg_options.inc"
[[ -f "$SRC" ]] || { echo "check_edg_options.sh: $SRC missing" >&2; exit 2; }

extract() {
  perl -0ne 'print "$1\n" while /add_option_description\(\s*optk_\w+,\s*"([^"]+)"/g' "$SRC"
}

# The options that cmd_line.c describes are not all compiled into the cerastes
# configuration (definition_list_file, for one, is under a #if); EDG resolves an
# abbreviation among those that are.  Ask the built front end: an option that is
# not compiled in is "invalid option".
FE="${CERASTES_FE:?build/env.sh does not set CERASTES_FE}"
[[ -x "$FE" ]] || { echo "check_edg_options.sh: $FE missing; run scripts/bootstrap.sh" >&2; exit 2; }
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
expected="$work/expected"
while IFS= read -r name; do
  out="$(cd "$work" && "$FE" "--$name" </dev/null 2>&1 || true)"
  if ! grep -q 'invalid option' <<<"$out"; then printf '    "%s",\n' "$name"; fi
done < <(extract) > "$expected"

if [[ "${1:-}" == "--update" ]]; then
  { awk '/^    "/ { exit } { print }' "$INC"; cat "$expected"; } > "$work/new.inc"
  cp "$work/new.inc" "$INC"
  echo "check_edg_options.sh: rewrote $INC ($(grep -c '^    "' "$INC") options)"
  exit 0
fi

if ! diff -u <(grep '^    "' "$INC") "$expected" >&2; then
  echo "check_edg_options.sh: src/driver/edg_options.inc does not match the options of $SRC that are compiled into the front end" >&2
  echo "  regenerate the option lines with the command in the header of $INC" >&2
  exit 1
fi
echo "edg options: $(grep -c '^    "' "$INC") options match $SRC (compiled into the front end)"
