// expect: error postcondition @ L32
// mode: c23, c++20
// twin: tests/memory/forall_max_index.c
// Pins: 0061 item 9 (twin): the loop stops one element early: the ensures over all n elements fails at the return
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool le_at(int k, const int *a, int n, int j)
{
  return !(0 <= k && k < n) || a[k] <= a[j];
}
CERASTES_PURE bool lt_at(int k, const int *a, int n, int j)
{
  return !(0 <= k && k < n) || a[k] < a[j];
}

int max_index(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && 0 <= r && r < n && cerastes_forall(le_at, a, n, r))
{
  int j = 0;
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= n && 0 <= j && j < i && cerastes_forall(le_at, a, i, j))
  CERASTES_DECREASES(n - i)
  while (i < n - 1) {
    if (a[i] > a[j]) {
      j = i;
    }
    i++;
  }
  return j;
}
