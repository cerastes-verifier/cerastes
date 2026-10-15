// expect: frontend-refused precompiled headers
// cerastes-fe-options: --c++20 --create_p=x.pch
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: an abbreviation of --create_pch is the same option to EDG.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
