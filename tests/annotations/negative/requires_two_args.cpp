// expect: frontend-error exp_rparen @ L9
// expect: frontend-error exp_comma @ L9
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: requires with two arguments: EDG's comma-free argument grammar stops at the comma (two parse errors, read from a run against the fork).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(x >= 0, x < 5);
