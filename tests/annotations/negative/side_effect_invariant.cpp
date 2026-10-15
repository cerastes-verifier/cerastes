// expect: frontend-error cerastes-annotation_side_effect @ L12
// expect: native-ok
// twin: tests/annotations/statements/invariant_while.cpp
// Mutation of the twin: assignment in a loop invariant.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int sum_to(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (s = 0) >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  return s;
}
