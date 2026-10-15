// expect: frontend-refused --undefine_macro of __CERASTES__ is not supported
// cerastes-fe-options: --c++20 --undefine_macro=__CERASTES__
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: the long form of -U.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
