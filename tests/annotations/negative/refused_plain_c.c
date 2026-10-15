// expect: frontend-refused C before C23 is not supported
// cerastes-fe-options: --c
// twin: tests/annotations/contracts/c_mode.c
// Mutation of the twin: plain --c selects the oldest C mode, which cannot carry a contract: PENDING, today it is not refused (the parser fails on the attribute instead).
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
