# EDG's cfe_main (edg_fe) calls back_end() (cerastes_backend), which calls back into EDG.  GNU ld reads each static
# archive once, left to right, so on Linux the two are a group it rescans (decision 0031 item 4); macOS's linker
# resolves the cycle itself.
if(APPLE)
  set(CERASTES_FE_LIBS cerastes_backend edg_fe)
  set(CERASTES_VERIFY_LIBS cerastes_backend cerastes_vir cerastes_viper edg_fe)
else()
  set(CERASTES_FE_LIBS "$<LINK_GROUP:RESCAN,cerastes_backend,edg_fe>")
  set(CERASTES_VERIFY_LIBS "$<LINK_GROUP:RESCAN,cerastes_backend,edg_fe>" cerastes_vir cerastes_viper)
endif()

add_executable(cerastes-fe main.cpp edg_args.cpp)
target_link_libraries(cerastes-fe PRIVATE ${CERASTES_FE_LIBS})
target_compile_features(cerastes-fe PRIVATE cxx_std_20)
target_compile_options(cerastes-fe PRIVATE -Wall -Wextra -Werror)

# `cerastes verify` (decision 0011).  The tool locations are fixed at configure time (0011 item 3): scripts/bootstrap.sh
# passes them once it has found them (it configures twice: the first time without them, to build cerastes-fe).
set(CERASTES_JAVA "" CACHE FILEPATH "java of JDK 17, for Silicon")
set(CERASTES_SILICON_JAR "" CACHE FILEPATH "silicon.jar")
set(CERASTES_Z3_EXE "" CACHE FILEPATH "the pinned Z3")
set(CERASTES_CLANG_VERSION "" CACHE STRING "the clang version EDG emulates (EDG's --clang_version)")
set(CERASTES_CXX_SYS_INCLUDES "" CACHE STRING "colon-separated C++ system include directories")
set(CERASTES_C_SYS_INCLUDES "" CACHE STRING "colon-separated C system include directories")
set(CERASTES_EDG_BASE "" CACHE PATH "EDG_BASE: the directory with EDG's lib/ data")
if(CERASTES_JAVA AND CERASTES_SILICON_JAR AND CERASTES_Z3_EXE AND CERASTES_CLANG_VERSION
   AND CERASTES_CXX_SYS_INCLUDES AND CERASTES_C_SYS_INCLUDES AND CERASTES_EDG_BASE)
  set(CERASTES_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
  configure_file(tool_paths.h.in "${CMAKE_CURRENT_BINARY_DIR}/tool_paths.h" @ONLY)
  add_executable(cerastes
    verify_main.cpp edg_args.cpp silicon.cpp silicon_output.cpp report.cpp pipeline.cpp selftest.cpp)
  target_include_directories(cerastes PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
  target_link_libraries(cerastes PRIVATE ${CERASTES_VERIFY_LIBS})
  target_compile_features(cerastes PRIVATE cxx_std_20)
  target_compile_options(cerastes PRIVATE -Wall -Wextra -Werror)
else()
  message(STATUS "cerastes verify: tool locations not given (CERASTES_JAVA, CERASTES_SILICON_JAR, "
                 "CERASTES_Z3_EXE, CERASTES_CLANG_VERSION, CERASTES_*_SYS_INCLUDES, CERASTES_EDG_BASE): target `cerastes` not defined")
endif()
