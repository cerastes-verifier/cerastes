// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 6 and 7 (S45 (d)): obligations of E at the site: a[i + 1] under i < n - 1 (n >= 1: n - 1 cannot overflow) stays in the range, and a[i] + 1 > 0 rests on an earlier quantified clause (a[i] < 100): no obligation fails
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool next_nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n - 1) || a[i + 1] >= 0;
}

CERASTES_PURE bool below100(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] < 100;
}

CERASTES_PURE bool succ_pos(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] + 1 > 0;
}

void shifted(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n >= 1 && cerastes_forall(next_nonneg, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}

void chained(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(below100, a, n) && cerastes_forall(succ_pos, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
