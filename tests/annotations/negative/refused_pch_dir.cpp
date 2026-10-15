// expect: frontend-refused precompiled headers
// cerastes-fe-options: --c++20 --pch_dir=pchdir
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: --pch_dir names a directory for precompiled headers.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
