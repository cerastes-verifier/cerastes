// expect: frontend-error wrong_entity_for_attribute @ L26
// expect: native-ok
// twin: tests/annotations/contracts/c_mode.c
// Mutation of the twin: requires on a statement, in C23.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

int abs_val(int x) CERASTES_REQUIRES(x > -1000) CERASTES_ENSURES(r, r >= 0);

int abs_val(int x)
{
  return x < 0 ? -x : x;
}

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

int count_to(int n) CERASTES_REQUIRES(n >= 0) CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) while (i < n) {
    i += 1;
  }
  CERASTES_REQUIRES(i == n);
  CERASTES_FOLD(nonneg(i));
  return i;
}
