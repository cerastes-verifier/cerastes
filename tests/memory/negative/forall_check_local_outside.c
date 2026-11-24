// expect: error out-of-bounds @ L18
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0061 item 10 (S45 (f) check_oob_fail): the quantifier reaches a[4] of a 4-element local array: the bound row fails (out-of-bounds), alone
// Pre-registered from record 0061 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int check_it(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  CERASTES_CHECK(cerastes_forall(nonneg, a, 5));
  (void)a[0];
  return 0;
}
