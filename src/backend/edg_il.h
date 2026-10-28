// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// The single place where EDG's IL headers are included.
//
// Rules for any translation unit that includes EDG headers:
//  1. It must link the `edg_config` CMake target.  That target is the only
//     place the EDG configuration macros (BACK_END_IS_*_GEN_BE,
//     DO_IL_LOWERING, PROTOTYPE_INSTANTIATIONS_IN_IL, ...) are defined; the
//     IL struct layouts depend on them, so a mismatch silently misreads the IL.
//  2. It must include "basics.h" BEFORE any other EDG header (il_def.h alone
//     fails with "unknown type name BEGIN_EDG_NAMESPACE").  Include this
//     header instead of the EDG ones.
//  3. EDG is built with USE_EDG_NAMESPACE effectively off here: all EDG names
//     (il_header, a_routine_ptr, ...) are global.
#pragma once

// fe_common.h is what every EDG translation unit includes first; it pulls in
// basics.h, il_def.h, il.h, il_to_str.h, mem_tables.h, types.h, ...  Including
// the individual headers by hand misses prerequisites (check_assertion,
// a_string_view, ...).
#include "basic_hdrs.h"   // first: basics.h, lang_feat.h, host_envir.h (rule 2)
#include "fe_common.h"
#include "il_walk.h"

// Mechanical guard of the configuration (decision 0002).  Each macro below is
// either set in cmake/EdgFrontEnd.cmake's edg_config or derived from it; if
// this TU was compiled without edg_config, or the derivation changed with a new
// EDG pin, the IL layout or content would silently differ.  The same values
// are checked on the built binary by scripts/check_edg_config.sh.
#define CERASTES_REQUIRE_(macro, value) \
  static_assert((macro) == (value), #macro " must be " #value " (see cmake/EdgFrontEnd.cmake)")
CERASTES_REQUIRE_(BACK_END_IS_C_GEN_BE, 0);
CERASTES_REQUIRE_(BACK_END_IS_CP_GEN_BE, 0);
CERASTES_REQUIRE_(DO_IL_LOWERING, 0);
CERASTES_REQUIRE_(IL_SHOULD_BE_WRITTEN_TO_FILE, 0);
CERASTES_REQUIRE_(PROTOTYPE_INSTANTIATIONS_IN_IL, 1);
CERASTES_REQUIRE_(ALL_TEMPLATE_INFO_IN_IL, 1);
CERASTES_REQUIRE_(RECORD_UNRECOGNIZED_ATTRIBUTES, 1);
CERASTES_REQUIRE_(RECORD_MACROS_IN_IL, 1);
CERASTES_REQUIRE_(EXTRA_SOURCE_POSITIONS_IN_IL, 1);
CERASTES_REQUIRE_(INSTANTIATE_TEMPLATES_EVERYWHERE_USED, 1);
CERASTES_REQUIRE_(DEFAULT_REMOVE_UNNEEDED_ENTITIES, 0);
CERASTES_REQUIRE_(MAINTAIN_NEEDED_FLAGS, 1);
CERASTES_REQUIRE_(SAME_REPR_INTS_INTERCHANGEABLE_IN_IL, 0);
CERASTES_REQUIRE_(FULLY_RESOLVED_MACRO_POSITIONS, 1);
CERASTES_REQUIRE_(INCLUDE_EDG_TEST_ATTRIBUTES, 0);
// M4.4b2 (decision 0024): file-scope static assertions exist in the IL only on the source sequence lists.
CERASTES_REQUIRE_(GENERATE_SOURCE_SEQUENCE_LISTS, 1);
// S5 (decision 0031 item 7): the same language on macOS and Linux.
CERASTES_REQUIRE_(EMBEDDED_C_ALLOWED, 0);
CERASTES_REQUIRE_(FIXED_POINT_ALLOWED, 0);
CERASTES_REQUIRE_(NAMED_ADDRESS_SPACES_ALLOWED, 0);
CERASTES_REQUIRE_(NAMED_REGISTERS_ALLOWED, 0);
CERASTES_REQUIRE_(DEFAULT_PASS_STDARG_REFERENCES_TO_GENERATED_CODE, 0);
// Derived:
CERASTES_REQUIRE_(AUTOMATIC_TEMPLATE_INSTANTIATION, 0);
CERASTES_REQUIRE_(FUNCTION_PROTOTYPE_INSTANTIATION_DEFERRAL_ALLOWED, 0);
// Turned on by GENERATE_SOURCE_SEQUENCE_LISTS (host_envir.h:1569-1581, 1734-1768); both change the IL layout.
CERASTES_REQUIRE_(GENERATE_LINKAGE_SPEC_BLOCKS, 1);
CERASTES_REQUIRE_(GENERATE_MICROSOFT_IF_EXISTS_ENTRIES, 1);
CERASTES_REQUIRE_(TEMPLATE_INSTANTIATIONS_IN_SOURCE_SEQUENCE_LISTS, 0);
static_assert(DEFAULT_INSTANTIATION_MODE == tim_used, "DEFAULT_INSTANTIATION_MODE must be tim_used");
// From the stock configuration (cmake_defines.h, macOS and Linux alike), not edg_config: the
// fix of EDG bug 0007 (internal/edg_bugs).  It adds type, attribute and token
// kinds (tk_vector, ...) and so changes the IL layout; libc++'s
// <__algorithm/simd_utils.h> needs it.
static_assert(GNU_VECTOR_TYPES_ALLOWED == 1, "GNU_VECTOR_TYPES_ALLOWED must be 1 (EDG bug 0007's fix)");
#undef CERASTES_REQUIRE_
