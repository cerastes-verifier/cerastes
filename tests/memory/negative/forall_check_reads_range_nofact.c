// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/forall_check_reads_range.c
// Pins: 0061 item 10 (twin): without the quantified requires the fact is not known: the fact row fails (check)
// Pre-registered from record 0061 item 10 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void keep_nonneg(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  CERASTES_CHECK(cerastes_forall(nonneg, a, n));
  (void)a;
  (void)n;
}
