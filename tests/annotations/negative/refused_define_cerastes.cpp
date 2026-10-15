// expect: frontend-refused -D of __CERASTES__ is not supported
// cerastes-fe-options: --c++20 -D__CERASTES__
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: the driver defines __CERASTES__ itself.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
