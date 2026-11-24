// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 7 (audit round 1 S12): a read the binder does not index (a[r] in a[i] <= a[r], r the result): a scalar ghost of today's form whose framing row is quantified over the binder with E's path; the maximum's index satisfies the ensures
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
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
  return a[0] >= a[1] ? 0 : 1;
}
