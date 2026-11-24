// expect: error invariant-preserved @ L19
// mode: c23, c++20
// twin: tests/memory/forall_sorted_loop.c
// Pins: 0061 item 9 (S45 (b) nbr_loop_fail): the comparison is reversed: the loop continues past an ascending pair and the neighbours clause of the invariant is not preserved (one error)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool sorted_at(int k, const int *a, int n)
{
  return !(0 <= k && k < n - 1) || a[k] <= a[k + 1];
}

bool is_sorted(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n >= 1)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (!r || cerastes_forall(sorted_at, a, n)))
{
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= n && cerastes_forall(sorted_at, a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (a[i - 1] < a[i]) {
      return false;
    }
    i++;
  }
  return true;
}
