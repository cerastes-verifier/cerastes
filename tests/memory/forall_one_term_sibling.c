// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 6 (question 253, S45 (b) addendum readq): the sibling of the one-term false alarm: the body also reads q[2], the other class's term at the same slot, so the multi-class quantifier is instantiated and p[2] >= 0 follows
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool same_at(int i, const int *p, const int *q, int n)
{
  return !(0 <= i && i < n) || p[i] == q[i];
}

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int via_p(const int *p, const int *q, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, n) && n > 2 && cerastes_forall(same_at, p, q, n) && cerastes_forall(nonneg, q, n))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && cerastes_reads_range(q, n) && r >= 0)
{
  (void)n;
  return p[2] + (q[2] - q[2]);
}
