// expect: frontend-error wrong_entity_for_attribute @ L9
// expect: native-ok
// twin: tests/annotations/contracts/requires_decl.cpp
// Mutation of the twin: a contract in a template-argument type-id (CP2a probe p12).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class F> struct holder {};
holder<int(int x) CERASTES_REQUIRES(x >= 0)> half;
