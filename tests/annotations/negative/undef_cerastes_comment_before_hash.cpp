// expect: frontend-error cerastes-cerastes_macro_changed @ L6
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a comment before the `#` of the directive.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
/* c */ #undef __CERASTES__
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0);
