// expect: frontend-error cerastes-contract_not_after_parameters @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: ensures written as a prefix.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_ENSURES(true) int half(int x);
