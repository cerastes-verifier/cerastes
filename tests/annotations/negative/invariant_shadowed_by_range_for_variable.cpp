// expect: frontend-error cerastes-invariant_shadowed_by_loop_variable @ L13
// expect: native-ok
// twin: tests/annotations/statements/invariant_range_for.cpp
// Mutation of the twin: the range-for declares v: the invariant would bind the outer v.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int total(void)
{
  const int a[3] = {1, 2, 3};
  int s = 0;
  [[maybe_unused]] int v = 0;
  CERASTES_INVARIANT(v >= 0 && s >= 0) for (int v : a) {
    s += v;
  }
  return s;
}
