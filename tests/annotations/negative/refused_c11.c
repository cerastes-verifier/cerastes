// expect: frontend-refused C before C23 is not supported
// cerastes-fe-options: --c11
// twin: tests/annotations/contracts/c_mode.c
// Mutation of the twin: C before C23: the driver refuses the option (twin: the C23 file).
// Pins: the driver refuses the options before EDG runs (exit 2, one line).
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
  CERASTES_CHECK(i == n);
  CERASTES_FOLD(nonneg(i));
  return i;
}
