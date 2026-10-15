// expect: frontend-error undefined_identifier @ L11
// expect: native-ok
// twin: tests/annotations/statements/invariant_for.cpp
// Mutation of the twin: a for-init variable is not in scope in the invariant (0006 item 2).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int count(int n) CERASTES_REQUIRES(n >= 0)
{
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i) for (int i = 0; i < n; ++i) {
    c += 1;
  }
  return c;
}
