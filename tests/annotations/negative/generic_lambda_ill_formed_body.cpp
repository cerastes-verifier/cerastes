// expect: frontend-error expr_not_class @ L8
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a generic lambda whose body is ill-formed for the argument: EDG's own instantiation error comes first (annotation_substitution_error has no reproducer yet).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int half(int x) CERASTES_REQUIRES([](auto y) { return y.foo(); }(1L));
