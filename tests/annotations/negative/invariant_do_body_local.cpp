// expect: frontend-error undefined_identifier @ L11
// expect: native-ok
// twin: tests/annotations/statements/invariant_do.cpp
// Mutation of the twin: a local of the do body is not in scope at the invariant.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int at_least_one(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(j >= 0 && i <= n + 1) do {
    [[maybe_unused]] int j = 0;
    i += 1;
  } while (i < n);
  return i;
}
