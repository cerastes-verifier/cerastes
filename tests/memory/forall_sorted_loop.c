// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 6 and 9 (S45 (b), T2): sortedness over neighbours in a loop: the quantified invariant over two read classes of one base (a[k], a[k + 1]), an early return of false, and the ensures `r implies sorted`
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
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
    if (a[i - 1] > a[i]) {
      return false;
    }
    i++;
  }
  return true;
}
