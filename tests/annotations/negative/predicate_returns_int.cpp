// expect: frontend-error cerastes-predicate_not_bool @ L8
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_pure.cpp
// Mutation of the twin: a predicate that does not return bool.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_PREDICATE int nonneg(int x)
{
  return x >= 0;
}

CERASTES_PURE int clamp_hi(int x, int hi)
{
  return x < hi ? x : hi;
}

CERASTES_PURE int dbl(int x) CERASTES_REQUIRES(x >= 0) CERASTES_ENSURES(r, r >= x)
{
  return x + x;
}
