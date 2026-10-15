// expect: frontend-error cerastes-contract_not_after_parameters @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: requires written as a prefix (it names no parameter, so EDG accepts it and cerastes diagnoses the placement).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_REQUIRES(true) int half(int x);
