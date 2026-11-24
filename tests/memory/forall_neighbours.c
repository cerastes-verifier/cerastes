// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 6 (S45 (b), T2): neighbours, a[i] <= a[i + 1] under 0 <= i && i < n - 1 (n >= 2: no overflow in n - 1): two read classes of one base, one trigger set over both terms; used by the callee at the slots it reads and proved by the caller on a sorted array
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool sorted_at(int i, const int *a, int n)
{
  return !(0 <= i && i < n - 1) || a[i] <= a[i + 1];
}

int ordered(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n >= 2 && cerastes_forall(sorted_at, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == 1)
{
  (void)n;
  return a[0] <= a[1];
}

int caller(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1, 2, 2, 5};
  return ordered(a, 4);
}
