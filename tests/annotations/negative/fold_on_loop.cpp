// expect: frontend-error attr_requires_null_statement @ L17
// expect: native-ok
// twin: tests/annotations/statements/fold_unfold.cpp
// Mutation of the twin: fold on a loop.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_PREDICATE bool in_range(int lo, int hi, int v)
{
  return lo <= v && v < hi;
}

int use([[maybe_unused]] int lo, [[maybe_unused]] int hi, int v) CERASTES_REQUIRES(in_range(lo, hi, v))
{
  CERASTES_UNFOLD(in_range(lo, hi, v));
  int w = v + 0;
  CERASTES_FOLD(in_range(lo, hi, w)) while (false) {}
  return w;
}
