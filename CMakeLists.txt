# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Builds the cerastes EDG front end (internal/decisions/0002-backend-integration.md):
# EDG's front-end sources compiled with the cerastes configuration, plus our
# in-process back end and driver.  Configure through scripts/bootstrap.sh
# (cmake -S . -B build/cmake); the generated headers come from the stock EDG
# build of the same tree (build/edg; see CERASTES_EDG_SRC in cmake/EdgFrontEnd.cmake).
cmake_minimum_required(VERSION 3.20)
project(cerastes LANGUAGES CXX)

set(CERASTES_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build" CACHE PATH
    "Where cerastes-fe is written (build/override for an EDG override, decision 0005)")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CERASTES_OUTPUT_DIR}")
# The vlibc checkout (decision 0046): third_party/vlibc, or CERASTES_VLIBC_SRC's tree; scripts/bootstrap.sh passes
# it as an absolute path.  Required like the tool locations of the driver: the back end's reserved names and the
# driver's include path come from it.
set(CERASTES_VLIBC_DIR "" CACHE PATH "the vlibc checkout (its include/ holds the shim and <vlibc/...>)")
if(NOT CERASTES_VLIBC_DIR OR NOT IS_DIRECTORY "${CERASTES_VLIBC_DIR}/include/vlibc")
  message(FATAL_ERROR "CERASTES_VLIBC_DIR must name a vlibc checkout with include/vlibc (scripts/bootstrap.sh passes it; "
                      "is the submodule third_party/vlibc initialized?); it is '${CERASTES_VLIBC_DIR}'")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # our sources: strict C++20; edg_fe overrides (EDG builds as gnu++14)

include(cmake/EdgFrontEnd.cmake)

add_subdirectory(src/vir)
add_subdirectory(src/viper)
add_subdirectory(src/backend)
add_subdirectory(src/driver)
