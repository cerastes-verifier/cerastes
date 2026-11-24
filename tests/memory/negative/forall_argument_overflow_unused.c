// expect: error overflow @ L15
// mode: c23, c++20
// twin: tests/memory/forall_argument_obligation.c
// Pins: 0061 item 3 (audit round 1 B1): x + 1 may overflow, and the parameter m of the pure function is never used: the argument's own obligation is a row at the site, outside the binder, reported whether or not f uses it
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_none(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void unbounded(const int *a, int n, int x)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(ge_none, a, n, x + 1))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
  (void)x;
}
