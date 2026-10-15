// expect: frontend-error cerastes-fold_argument_not_call @ L17
// expect: native-ok
// twin: tests/annotations/statements/fold_unfold.cpp
// Mutation of the twin: fold of something that is not a call.
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
  CERASTES_FOLD(w);
  return w;
}
