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

add_library(cerastes_backend STATIC
  back_end.cpp options.cpp register_attributes.cpp run_state.cpp il_names.cpp render.cpp
  walker.cpp "${IL_NAMES_INC}")
target_include_directories(cerastes_backend PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(cerastes_backend PUBLIC edg_config)
target_compile_features(cerastes_backend PUBLIC cxx_std_20)
target_compile_options(cerastes_backend PRIVATE -Wall -Wextra -Werror)
