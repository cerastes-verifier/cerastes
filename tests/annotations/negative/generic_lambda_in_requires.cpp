// expect: frontend-error cerastes-generic_lambda_in_annotation @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a generic lambda in a contract: its call operator is never checked.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES([](auto i) { return i >= 0; }(x));
