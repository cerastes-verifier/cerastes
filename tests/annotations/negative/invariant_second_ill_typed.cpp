// expect: frontend-error expr_not_class @ L13
// expect: native-ok
// twin: tests/annotations/statements/invariant_two_nested.cpp
// Mutation of the twin: the second invariant of a chain is checked too.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int grid(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  int j = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_INVARIANT(c.size >= 0) while (i < n) {
    j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n) while (j < n) {
      c += 1;
      j += 1;
    }
    i += 1;
  }
  return c;
}
