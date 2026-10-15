// expect: frontend-error attr_requires_null_statement @ L12
// expect: native-ok
// twin: tests/annotations/statements/invariant_while.cpp
// Mutation of the twin: check on a loop instead of a null statement.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int sum_to(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  int s = 0;
  CERASTES_CHECK(0 <= i && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  return s;
}
