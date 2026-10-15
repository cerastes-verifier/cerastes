// expect: frontend-error attr_requires_iteration_statement @ L19
// expect: native-ok
// twin: tests/annotations/statements/check_null.cpp
// Mutation of the twin: invariant on a compound statement.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int pick(int a, int b) CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  CERASTES_CHECK(a + b >= 0);
  int m;
  if (a < b) {
    CERASTES_CHECK(a < b);
    m = a;
  } else {
    CERASTES_CHECK(a >= b);
    m = b;
  }
  CERASTES_INVARIANT(m <= a) {
    CERASTES_CHECK(m <= a && m <= b);
  }
  return m;
}
