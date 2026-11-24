// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/forall_unindexed_read.c
// Pins: 0061 item 7 (twin): the branches are swapped, so the returned index names the smaller element: the quantified ensures fails at the return
// Pre-registered from record 0061 item 7 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool le_at(int i, const int *a, int n, int r)
{
  return !(0 <= i && i < n) || a[i] <= a[r];
}

int pick_max2(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n == 2)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && 0 <= r && r < n && cerastes_forall(le_at, a, n, r))
{
  (void)n;
  return a[0] >= a[1] ? 1 : 0;
}
