// expect: error check @ L19
// mode: c23, c++20
// twin: tests/memory/forall_check_conjunct.c
// Pins: 0061 item 9 and question 262 (audit round 3): the quantified conjunct of a check is checked after the plain one: a[1] is negative, n may be 2 or more, and the check fails (every row before the fact holds)
// Pre-registered from record 0061 item 9 (audit round 3, question 262), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int k, const int *a, int n)
{
  return !(0 <= k && k < n) || a[k] >= 0;
}

int check_it(int n)
    CERASTES_REQUIRES(0 <= n && n <= 4)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, -2, 3, 4};
  CERASTES_CHECK(0 <= n && cerastes_forall(nonneg, a, n));
  (void)a[0];
  (void)n;
  return 0;
}
