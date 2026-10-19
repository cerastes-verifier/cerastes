// expect: frontend-error cerastes-spec_call_in_code @ L20
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_calls_in_contracts.cpp
// Mutation of the twin: the initializer of a block-scope static variable calls a predicate.
// Pins: plan R4b: spec_call_in_code in the initializer of a block-scope static variable
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

int ident(int x) CERASTES_REQUIRES(nonneg(x)) CERASTES_ENSURES(r, r == x)
{
  return x;
}

int use_static(int x)
{
  static int cached = nonneg(1);
  return x + cached;
}
