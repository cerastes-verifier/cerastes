// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 9 (S45 (c)): a fill loop over an owns_range parameter: the quantified invariant (every element below i is zero) gives the quantified ensures at the loop's end (the loop carries the range's places)
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void zero(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_forall(zero_at, p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(zero_at, p, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i] = 0;
    i++;
  }
}
