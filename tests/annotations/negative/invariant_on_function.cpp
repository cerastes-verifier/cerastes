// expect: frontend-error wrong_entity_for_attribute @ L8
// expect: native-ok
// twin: tests/annotations/statements/invariant_while.cpp
// Mutation of the twin: invariant on a function instead of a loop.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int sum_to(int n) CERASTES_INVARIANT(n >= 0)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  return s;
}
