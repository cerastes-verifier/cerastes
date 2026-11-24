// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 4 and 9 (S45 (c) must_init; question 239): the sibling of range_marker_pin_loop_filler.c: a must_init filler with the invariant that the elements below i are zero (the marked parameter's element read under the binder in an invariant is accepted and carries the flag row): the loop's end gives back every flag, so the must_init return row holds
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void fill(int *p CERASTES_MUST_INIT, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(zero_at, p, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    p[i] = 0;
    i++;
  }
}
