// expect: frontend-error undefined_identifier @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: an undeclared name in a contract.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(y >= 0);
