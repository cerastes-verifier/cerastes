// expect: error precondition @ L26
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 7 (S45 (a) twin): a negative element in the caller's array: the precondition row of the quantified clause fails at the call
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int first(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0 && cerastes_forall(nonneg, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r >= 0)
{
  (void)n;
  return a[0];
}

int caller(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[3] = {1, -2, 3};
  return first(a, 3);
}
