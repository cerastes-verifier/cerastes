// expect: frontend-error undefined_identifier @ L12
// expect: native-ok
// twin: tests/annotations/statements/invariant_range_for.cpp
// Mutation of the twin: the range-for variable is not in scope in the invariant.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int total(void)
{
  const int a[3] = {1, 2, 3};
  int s = 0;
  CERASTES_INVARIANT(v >= 0) for (int v : a) {
    s += v;
  }
  return s;
}
