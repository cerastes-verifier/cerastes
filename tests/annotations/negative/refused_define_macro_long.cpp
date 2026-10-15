// expect: frontend-refused --define_macro of __CERASTES__ is not supported
// cerastes-fe-options: --c++20 --define_macro=__CERASTES__=0
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: the long form of -D.
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
