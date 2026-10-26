# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# edg_config: the ONE place that defines the EDG configuration macros.  Every
# target that includes an EDG header must link it, because the IL struct
# layout depends on these macros (decision 0002, "Layout safety").
# edg_fe: EDG's front-end sources (the ones that make up cpfe) as a library.

# The EDG tree to compile and the stock build that generated its headers.  The
# defaults are the pinned submodule and build/edg; scripts/bootstrap.sh passes
# the CERASTES_EDG_SRC override explicitly (decision 0005).
set(CERASTES_EDG_SRC "${CMAKE_SOURCE_DIR}/third_party/edg" CACHE PATH
    "Root of the EDG checkout (the directory that contains src/)")
set(CERASTES_EDG_GEN "${CMAKE_SOURCE_DIR}/build/edg/src/includes" CACHE PATH
    "Generated headers (err_codes.h, err_data.h, cmake_defines.h) of a stock build of CERASTES_EDG_SRC")
set(EDG_SRC "${CERASTES_EDG_SRC}/src")
set(EDG_GEN "${CERASTES_EDG_GEN}")

if(NOT EXISTS "${EDG_SRC}/il_def.h")
  message(FATAL_ERROR "CERASTES_EDG_SRC=${CERASTES_EDG_SRC} has no src/il_def.h")
endif()

foreach(f cmake_defines.h err_codes.h err_data.h)
  if(NOT EXISTS "${EDG_GEN}/${f}")
    message(FATAL_ERROR
      "missing ${EDG_GEN}/${f}: run scripts/bootstrap.sh (it builds the stock "
      "EDG, which generates the headers cerastes reuses)")
  endif()
endforeach()

# The generated headers must come from a stock build of this very tree: a
# fork may add diagnostics, and a mismatch would silently misnumber them.
set(_gen_cache "${EDG_GEN}/../../CMakeCache.txt")
if(NOT EXISTS "${_gen_cache}")
  message(FATAL_ERROR "${EDG_GEN} is not inside a stock EDG build (no ${_gen_cache})")
endif()
file(STRINGS "${_gen_cache}" _gen_home REGEX "^CMAKE_HOME_DIRECTORY:INTERNAL=")
string(REGEX REPLACE "^CMAKE_HOME_DIRECTORY:INTERNAL=" "" _gen_home "${_gen_home}")
file(REAL_PATH "${_gen_home}" _gen_home_real)
file(REAL_PATH "${CERASTES_EDG_SRC}" _edg_src_real)
if(NOT _gen_home_real STREQUAL _edg_src_real)
  message(FATAL_ERROR
    "${EDG_GEN} was generated from ${_gen_home_real}, but CERASTES_EDG_SRC is "
    "${_edg_src_real}; run scripts/bootstrap.sh (decision 0005)")
endif()

add_library(edg_config INTERFACE)
target_include_directories(edg_config SYSTEM INTERFACE "${EDG_SRC}" "${EDG_GEN}")
target_compile_definitions(edg_config INTERFACE
  BACK_END_IS_C_GEN_BE=0
  BACK_END_IS_CP_GEN_BE=0
  DO_IL_LOWERING=0
  IL_SHOULD_BE_WRITTEN_TO_FILE=0
  PROTOTYPE_INSTANTIATIONS_IN_IL=1
  ALL_TEMPLATE_INFO_IN_IL=1
  RECORD_UNRECOGNIZED_ATTRIBUTES=1
  # R4c (decision 0008 item 2): every #define and #undef of the translation unit is an IL entry (a_macro),
  # so a change of the names the annotations depend on is found from what the preprocessor really did.
  RECORD_MACROS_IN_IL=1
  EXTRA_SOURCE_POSITIONS_IN_IL=1
  INSTANTIATE_TEMPLATES_EVERYWHERE_USED=1
  DEFAULT_INSTANTIATION_MODE=tim_used
  # Load-bearing settings that would otherwise be inherited from derived
  # defaults or the stock-generated cmake_defines.h.  The default for this back
  # end is TRUE (host_envir.h:1138-1147), which prunes unused entities such as
  # never-instantiated templates; the walker needs them (G1 check 2).
  DEFAULT_REMOVE_UNNEEDED_ENTITIES=0
  MAINTAIN_NEEDED_FLAGS=1
  SAME_REPR_INTS_INTERCHANGEABLE_IN_IL=0
  # M2 (0002 amendment): diagnostics inside CERASTES_* macro arguments point at
  # the argument's own line (layout-affecting: adds orig_seq/orig_column), and
  # EDG's test attributes (a registered edg:: namespace once 0007 lands) stay
  # out of the cerastes front end.
  FULLY_RESOLVED_MACRO_POSITIONS=1
  INCLUDE_EDG_TEST_ATTRIBUTES=0
  # M4.4b2 (decision 0024, amending 0002): the file scope's source_sequence_list is the only IL record of a
  # file-scope _Static_assert / static_assert (block scope has the iek_static_assertion statement entity).  It
  # also turns on GENERATE_LINKAGE_SPEC_BLOCKS and GENERATE_MICROSOFT_IF_EXISTS_ENTRIES (host_envir.h:1734-1768)
  # and adds fields to the variable, routine, type, statement and source-correspondence structs (layout-affecting).
  GENERATE_SOURCE_SEQUENCE_LISTS=1
  EDG_MAIN=edg_main
  USE_CMAKE_DEFINES)

# Mirrors CORE_FRONT_END_SOURCE_FILES and the add_src_file_if_present list in
# third_party/edg/src/CMakeLists.txt; follow it when the pin moves.
set(EDG_FE_FILES
    attribute.c c_gen_be.c cfe.c class_decl.c cmd_line.c const_ints.c
    cp_gen_be.c debug.c decl_inits.c decl_spec.c declarator.c decls.c def_arg.c
    disambig.c error.c expr.c exprutil.c extasm.c fe_init.c fe_wrapup.c
    fixed_pt.c float_pt.c floating.c folding.c func_def.c host_envir.c
    ifc_modules.c il.c il_alloc.c il_display.c il_read.c il_to_str.c il_walk.c
    il_write.c inline.c interpret.c layout.c lexical.c literals.c lookup.c
    lower_c99.c lower_eh.c lower_il.c lower_init.c lower_name.c macro.c
    mem_manage.c modules.c ms_attrib.c overload.c pch.c pragma.c preproc.c
    scope_stk.c src_seq.c statements.c symbol_ref.c symbol_tbl.c sys_predef.c
    target.c templates.c trans_copy.c trans_corresp.c trans_unit.c types.c)
foreach(f cfe_daemon.c ifc_map_functions.c ifc_map_functions_acc.c
          ifc_map_functions_dbg.c ifc_map_functions_mut.c
          ifc_map_functions_val.c ifc_modules_read.c ifc_modules_templ.c
          ifc_modules_write.c unicode_name_fsm.c)
  if(EXISTS "${EDG_SRC}/${f}")
    list(APPEND EDG_FE_FILES ${f})
  endif()
endforeach()
list(TRANSFORM EDG_FE_FILES PREPEND "${EDG_SRC}/")
set_source_files_properties(${EDG_FE_FILES} PROPERTIES LANGUAGE CXX)

add_library(edg_fe STATIC ${EDG_FE_FILES})
target_link_libraries(edg_fe PUBLIC edg_config)
set_target_properties(edg_fe PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON
                      CXX_EXTENSIONS OFF)
# Exactly stock cpfe's flags (build/edg/build.ninja, any cpfe TU), so only the
# configuration macros differ.  No -Werror on EDG sources besides stock's
# -Werror=shadow.
target_compile_options(edg_fe PRIVATE
  -g3 -Wall -Wwrite-strings -Wformat-security -pedantic -Werror=shadow
  -Wimplicit-fallthrough -Wconversion -Wsign-conversion
  -fno-exceptions -fno-unwind-tables -fno-rtti -x c++)
