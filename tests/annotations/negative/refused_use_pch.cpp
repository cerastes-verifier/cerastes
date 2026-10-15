// expect: frontend-refused precompiled headers
// cerastes-fe-options: --c++20 --use_pch=x.pch
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: the same for --use_pch.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
