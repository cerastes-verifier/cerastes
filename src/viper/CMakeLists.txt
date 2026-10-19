# Viper text, the prelude and the position map (internal/encoding.md sections 4 and 5): printing only.
# No EDG headers, no edg_config.
add_library(cerastes_viper STATIC emit.cpp prelude.cpp posmap.cpp)
target_link_libraries(cerastes_viper PUBLIC cerastes_vir)
target_include_directories(cerastes_viper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(cerastes_viper PRIVATE -Wall -Wextra -Werror)

# The self-test: one VIR program per lowering rule, emitted as .vpr (+ .map) with `// expect:` headers for
# scripts/run_tests.py, and the typing-rule rejections checked in-process.  Usage: cerastes-vir-selftest <dir>.
add_executable(cerastes-vir-selftest selftest.cpp)
target_link_libraries(cerastes-vir-selftest PRIVATE cerastes_viper)
target_compile_options(cerastes-vir-selftest PRIVATE -Wall -Wextra -Werror)
