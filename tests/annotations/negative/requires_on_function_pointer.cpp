// expect: frontend-error wrong_entity_for_attribute @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a contract on a function pointer variable.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int (*half)(int x) CERASTES_REQUIRES(x >= 0);
