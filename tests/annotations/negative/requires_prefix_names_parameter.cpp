// expect: frontend-error undefined_identifier @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: the usual prefix mistake: x does not exist yet, so the user sees EDG's undefined identifier, not the placement diagnostic (a documentation point for the user guide).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_REQUIRES(x >= 0) int half(int x);
