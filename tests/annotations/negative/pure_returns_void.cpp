// expect: frontend-error cerastes-pure_returns_void @ L18
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_pure.cpp
// Mutation of the twin: a pure function that returns void.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

CERASTES_PURE int clamp_hi(int x, int hi)
{
  return x < hi ? x : hi;
}

CERASTES_PURE void nothing(int x);

CERASTES_PURE int dbl(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r >= x)
{
  return x + x;
}
