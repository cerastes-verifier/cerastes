// expect: frontend-refused precompiled headers
// cerastes-fe-options: --c++20 --pch
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: --pch (automatic precompiled headers) is refused too.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
