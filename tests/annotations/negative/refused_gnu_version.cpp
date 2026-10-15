// expect: frontend-refused GNU emulation
// cerastes-fe-options: --gnu_version=150000
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: GNU emulation: the driver reads C++ in clang emulation only.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
