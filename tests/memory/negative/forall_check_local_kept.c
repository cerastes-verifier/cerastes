// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/forall_check_local.c
// Pins: 0061 item 10 (S45 (f) check_kept_fail): a callee took a[1..4) and kept it: the marked row of the check fails (permission), one error: a failed marked row ends its path, so the scope exit is not reached
// Pre-registered from record 0061 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void take(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n));

int check_it(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[4] = {1, 2, 3, 4};
  take(&a[1], 3);
  CERASTES_CHECK(cerastes_forall(nonneg, a, 4));
  return 0;
}
