# Enumerator-name tables generated from the pinned il_def.h.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(IL_NAMES_INC "${CMAKE_CURRENT_BINARY_DIR}/il_names.inc")
add_custom_command(
  OUTPUT "${IL_NAMES_INC}"
  COMMAND Python3::Interpreter "${CMAKE_SOURCE_DIR}/cmake/gen_enum_names.py"
          "${EDG_SRC}/il_def.h" "${IL_NAMES_INC}"
          an_expr_operator_kind an_expr_node_kind a_type_kind a_statement_kind
          a_template_param_constant_kind a_constant_repr_kind
          a_template_param_type_kind
  DEPENDS "${CMAKE_SOURCE_DIR}/cmake/gen_enum_names.py" "${EDG_SRC}/il_def.h"
  COMMENT "Generating il_names.inc")

# The names the pinned vlibc headers give a contract (decision 0046 item 6: reserved names), generated at configure
# time into vlibc_names.inc (a fragment of sorted {name, header, paired} initializers, `paired` the standard name a
# `vlibc_` entry is paired with and "" for a standard name; encode.cpp includes it, the build directory is on the back
# end's private include path).  The script rewrites the file only when its content changes.  A pin move
# changes the headers, which re-runs the configure (CMAKE_CONFIGURE_DEPENDS); a script failure fails the configure.
set(VLIBC_SHIM_DIR "${CERASTES_VLIBC_DIR}/include/vlibc")
set(VLIBC_NAMES_INC "${CMAKE_CURRENT_BINARY_DIR}/vlibc_names.inc")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/vlibc_contracts.py" names
          --include-dir "${VLIBC_SHIM_DIR}" --out "${VLIBC_NAMES_INC}"
  RESULT_VARIABLE VLIBC_NAMES_STATUS
  OUTPUT_VARIABLE VLIBC_NAMES_OUTPUT
  ERROR_VARIABLE VLIBC_NAMES_OUTPUT)
if(NOT VLIBC_NAMES_STATUS EQUAL 0)
  message(FATAL_ERROR "scripts/vlibc_contracts.py names failed for ${VLIBC_SHIM_DIR} (status ${VLIBC_NAMES_STATUS}):\n${VLIBC_NAMES_OUTPUT}")
endif()
file(GLOB_RECURSE VLIBC_HEADERS CONFIGURE_DEPENDS "${CERASTES_VLIBC_DIR}/include/*")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             ${VLIBC_HEADERS} "${CMAKE_SOURCE_DIR}/scripts/vlibc_contracts.py")

add_library(cerastes_backend STATIC
  back_end.cpp options.cpp register_attributes.cpp run_state.cpp il_names.cpp render.cpp
  expr_tree.cpp diagnostics.cpp annotation_checks.cpp walker.cpp
  il_query.cpp verify_config.cpp encode.cpp "${IL_NAMES_INC}")
target_include_directories(cerastes_backend PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(cerastes_backend PUBLIC edg_config cerastes_vir cerastes_viper)
# The checkout's own include/ (it holds the real <cerastes.h>): the one file whose #define and #undef of the
# names the annotations depend on are not an error (annotation_macro_changed, decision 0008 item 2).
target_compile_definitions(cerastes_backend PRIVATE CERASTES_SOURCE_INCLUDE_DIR="${CMAKE_SOURCE_DIR}/include")
target_compile_features(cerastes_backend PUBLIC cxx_std_20)
target_compile_options(cerastes_backend PRIVATE -Wall -Wextra -Werror)
