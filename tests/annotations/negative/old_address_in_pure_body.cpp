// expect: frontend-error cerastes-old_outside_ensures @ L25
// expect: native-ok
// twin: tests/annotations/spec_functions/predicate_pure.cpp
// Mutation of the twin: a pure function takes the address of cerastes::old.
// Pins: plan R4e: old_outside_ensures in the body of a pure or predicate function (a call, its address, a dependent name)
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
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

CERASTES_PURE int addr(int x)
{
  [[maybe_unused]] auto f = &cerastes::old<int>;
  return x;
}
