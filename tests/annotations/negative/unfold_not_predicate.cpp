// expect: frontend-error cerastes-fold_callee_not_predicate @ L17
// expect: native-ok
// twin: tests/annotations/statements/fold_unfold.cpp
// Mutation of the twin: unfold of a call to a function that is not a predicate.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

CERASTES_PREDICATE bool in_range(int lo, int hi, int v)
{
  return lo <= v && v < hi;
}

inline bool plain(int v) { return v >= 0; }

int use([[maybe_unused]] int lo, [[maybe_unused]] int hi, int v) CERASTES_REQUIRES(in_range(lo, hi, v))
{
  CERASTES_UNFOLD(plain(v));
  int w = v + 0;
  CERASTES_FOLD(in_range(lo, hi, w));
  return w;
}
