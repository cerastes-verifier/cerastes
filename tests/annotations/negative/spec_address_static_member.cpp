// expect: frontend-error cerastes-spec_call_in_code @ L14
// expect: frontend-error cerastes-spec_call_in_code @ L15
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_pure.cpp
// Mutation of the twin: the address of a predicate in a static constexpr data member.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

struct holder { static constexpr bool (*pp)(int) = &nonneg; };
int call_pp(int x) { return holder::pp(x); }

CERASTES_PURE int clamp_hi(int x, int hi)
{
  return x < hi ? x : hi;
}

CERASTES_PURE int dbl(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r >= x)
{
  return x + x;
}
