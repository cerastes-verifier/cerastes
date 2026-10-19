// expect: frontend-error cerastes-spec_call_in_code @ L20
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_calls_in_contracts.cpp
// Mutation of the twin: the address of a predicate is a non-type template argument.
// Pins: plan R4b: spec_call_in_code for a predicate named as a function-pointer template argument
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

int ident(int x) CERASTES_REQUIRES(nonneg(x)) CERASTES_ENSURES(r, r == x)
{
  return x;
}

template <bool (*F)(int)> struct V { int v = 0; };

V<&nonneg> g_v;
