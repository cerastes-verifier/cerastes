// expect: error permission @ L20
// expect: error out-of-bounds @ L20
// mode: c23, c++20
// twin: tests/memory/forall_fill_local.c
// Pins: 0061 item 9 (audit round 1 N14, question 254): the quantified read reaches a[3] of a 3-element named array: the framing row (permission) and the bound row (out-of-bounds) report it; the flag line takes the bound in its antecedent, so it does not also fail (expected count: two, to be corrected after the first run if Silicon gives another)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

int outside(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0, 0, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(zero_at, a, 4))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i++;
  }
  (void)a[0];
  return 0;
}
