// expect: frontend-refused -U of __CERASTES__ is not supported
// cerastes-fe-options: --c++20 '-U __CERASTES__'
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: -U of a blank-padded __CERASTES__ is the same macro and is refused.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
