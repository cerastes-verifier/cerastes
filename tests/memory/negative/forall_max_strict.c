// expect: error postcondition @ L32
// mode: c23, c++20
// twin: tests/memory/forall_max.c
// Pins: 0061 item 9 (S45 (c) max_strict_fail): `<` for `<=` in the ensures: the maximum itself is not below it (one error at the return)
// Pre-registered from record 0061 item 9 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool le_m(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] <= m;
}
CERASTES_PURE bool lt_m(int k, const int *a, int n, int m)
{
  return !(0 <= k && k < n) || a[k] < m;
}

int max(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && cerastes_forall(lt_m, a, n, r))
{
  int m = a[0];
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= n && cerastes_forall(le_m, a, i, m))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (a[i] > m) {
      m = a[i];
    }
    i++;
  }
  return m;
}
