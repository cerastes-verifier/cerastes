// expect: frontend-error cerastes-annotation_side_effect @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a read of a volatile object in a contract.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(volatile int* p) CERASTES_REQUIRES(*p >= 0);
