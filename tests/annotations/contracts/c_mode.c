// expect: golden
// expect: native-ok
// Pins: C23: the same macros give the same attributes in C (requires is not a
// keyword there); contracts, loop invariant, check, a predicate and fold.
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
