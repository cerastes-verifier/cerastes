// expect: frontend-error cerastes-statement_expression_in_annotation @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a GNU statement expression in a contract (EDG's clang emulation accepts it; it must not name a parameter, or param_not_allowed comes first).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES(({ int t = 1; t > 0; }));
