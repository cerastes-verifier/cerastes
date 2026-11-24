// expect: rejected cerastes-unsupported_annotation @ L19
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 4: a call as a trailing argument of cerastes_forall is rejected (M4.5e2), at the call
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int cnt(int x)
{
  return x;
}

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int use(const int *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n) && cerastes_forall(nonneg, a, cnt(n)))
{
  (void)a;
  return n;
}
