# VIR: the front-end-agnostic verification IR, its type checker and its lowering to Viper-level form
# (internal/encoding.md).  No EDG headers, no edg_config.
add_library(cerastes_vir STATIC vir.cpp check.cpp lowered.cpp lower.cpp)
target_include_directories(cerastes_vir PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(cerastes_vir PUBLIC cxx_std_20)
target_compile_options(cerastes_vir PRIVATE -Wall -Wextra -Werror)
