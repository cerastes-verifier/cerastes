// expect: frontend-error cerastes-old_outside_ensures @ L10
// expect: native-ok
// twin: tests/annotations/statements/check_null.cpp
// Mutation of the twin: old() in a check.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int pick(int a, int b) CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  CERASTES_CHECK(a + b >= cerastes::old(a));
  int m;
  if (a < b) {
    CERASTES_CHECK(a < b);
    m = a;
  } else {
    CERASTES_CHECK(a >= b);
    m = b;
  }
  {
    CERASTES_CHECK(m <= a && m <= b);
  }
  return m;
}
