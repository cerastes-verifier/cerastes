#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Fail unless build/cerastes-fe reports the EDG configuration cerastes needs
# (decision 0002, cmake/EdgFrontEnd.cmake).  The same values are asserted at
# compile time in src/backend/edg_il.h; this checks the built binary itself.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
[[ -f "$ROOT/build/env.sh" ]] || { echo "check_edg_config.sh: run scripts/bootstrap.sh first" >&2; exit 2; }
# shellcheck source=/dev/null
. "$ROOT/build/env.sh"
FE="$CERASTES_FE"   # the binary of the active EDG tree (decision 0005)
[[ -x "$FE" ]] || { echo "check_edg_config.sh: $FE missing; run scripts/bootstrap.sh" >&2; exit 2; }

config="$("$FE" --dump_configuration 2>&1)"
fail=0
require() {
  local macro="$1" want="$2" got
  got="$(awk -v m="$macro" '$1 == "#define" && $2 == m { print $3 }' <<<"$config")"
  if [[ "$got" != "$want" ]]; then
    echo "edg configuration: $macro is '${got:-<undefined>}', expected '$want'" >&2
    fail=1
  fi
}

require BACK_END_IS_C_GEN_BE 0
require BACK_END_IS_CP_GEN_BE 0
require DO_IL_LOWERING 0
require IL_SHOULD_BE_WRITTEN_TO_FILE 0
require PROTOTYPE_INSTANTIATIONS_IN_IL 1
require ALL_TEMPLATE_INFO_IN_IL 1
require RECORD_UNRECOGNIZED_ATTRIBUTES 1
require EXTRA_SOURCE_POSITIONS_IN_IL 1
require INSTANTIATE_TEMPLATES_EVERYWHERE_USED 1
require DEFAULT_INSTANTIATION_MODE tim_used
require DEFAULT_REMOVE_UNNEEDED_ENTITIES 0
require MAINTAIN_NEEDED_FLAGS 1
require SAME_REPR_INTS_INTERCHANGEABLE_IN_IL 0
require FULLY_RESOLVED_MACRO_POSITIONS 1
require INCLUDE_EDG_TEST_ATTRIBUTES 0
require AUTOMATIC_TEMPLATE_INSTANTIATION 0
require FUNCTION_PROTOTYPE_INSTANTIATION_DEFERRAL_ALLOWED 0

if [[ $fail -ne 0 ]]; then
  exit 1
fi
echo "edg configuration: ok"
