// expect: error overflow @ L15
// mode: c23, c++20
// twin: tests/memory/forall_argument_obligation.c
// Pins: 0061 item 3 (audit round 1 B1): m is used only where i < 0 and 0 <= i, a path no i reaches: the overflow of x + 1 is still reported at the site
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_never(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || (i < 0 && a[i] >= m);
}

void unbounded(const int *a, int n, int x)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(ge_never, a, n, x + 1))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
  (void)x;
}
