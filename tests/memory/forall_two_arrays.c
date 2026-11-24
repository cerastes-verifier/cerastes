// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 6 (S45 (b), T2): two arrays, a[i] == b[i]: two read classes of two bases (a binder per class, one trigger set over both terms); the callee uses it at the slots it reads (the difference is zero), the caller proves it for equal arrays
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool same_at(int i, const int *a, const int *b, int n)
{
  return !(0 <= i && i < n) || a[i] == b[i];
}

int diff(const int *a, const int *b, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_reads_range(b, n) && n > 0 && cerastes_forall(same_at, a, b, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_reads_range(b, n) && r == 0)
{
  (void)n;
  return a[0] - b[0];
}

int caller(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  int b[3] = {1, 2, 3};
  return diff(a, b, 3);
}
