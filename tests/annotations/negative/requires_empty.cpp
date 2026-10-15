// expect: frontend-error exp_primary_expr @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: requires with no argument: EDG expects an expression (read from a run against the fork).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES();
