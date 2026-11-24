// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/forall_check_owns_range.c
// Pins: 0061 item 10 (S45 (f) check_owns_nofact_fail): without the quantified requires the fact is not known: every row before it holds and the fact row fails (check)
// Pre-registered from record 0061 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void keep_nonneg(int *a, int n)
    CERASTES_REQUIRES(cerastes_owns_range(a, n))
    CERASTES_ENSURES(cerastes_owns_range(a, n))
{
  CERASTES_CHECK(cerastes_forall(nonneg, a, n));
  (void)a;
  (void)n;
}
