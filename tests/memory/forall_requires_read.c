// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 items 3, 6 and 7 (S45 (a)): a callee whose requires says every element is non-negative (a pure function named by cerastes_forall) and that reads one, and its caller on a local array: the precondition row after the atom's rows, the callee's ensures used
// Pre-registered from record 0061 item 7 (Part B), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

int first(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && n > 0 && cerastes_forall(nonneg, a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r >= 0)
{
  (void)n;
  return a[0];
}

int caller(void)
    CERASTES_ENSURES(r, r >= 0)
{
  int a[3] = {1, 2, 3};
  return first(a, 3);
}
