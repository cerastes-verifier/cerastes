// expect: frontend-error cerastes-cerastes_macro_changed @ L6
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: the splice inside the macro name.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#undef __CERAS\
TES__
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
