// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 6 and 9 (S45 (b), T2): a copy loop over two ranges: the quantified invariant dst[k] == src[k] for k below i (two read classes of two bases, one trigger set over both terms) gives the quantified ensures
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool same_at(int k, const int *a, const int *b, int n)
{
  return !(0 <= k && k < n) || a[k] == b[k];
}

void copy(int *dst, const int *src, int n)
    CERASTES_REQUIRES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n))
    CERASTES_ENSURES(cerastes_owns_range(dst, n) && cerastes_reads_range(src, n) && cerastes_forall(same_at, dst, src, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(same_at, dst, src, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    dst[i] = src[i];
    i++;
  }
}
