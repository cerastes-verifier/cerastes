// expect: error overflow @ L15
// mode: c23, c++20
// twin: tests/memory/forall_site_obligations.c
// Pins: 0061 items 6 and 7 (S45 (d) twin): a[i] + 1 can overflow when no earlier clause bounds a[i]: the overflow of E is reported at the forall's clause
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool succ_pos(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] + 1 > 0;
}

void chained(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(succ_pos, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
