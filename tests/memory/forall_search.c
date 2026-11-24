// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0061 item 9 (S45 (c) search): a search loop with a return in its body: the progress invariant (no element below i equals x) gives `r == -1` implies no element equals x; the trailing argument x is a parameter (no obligation)
// Pre-registered from record 0061 item 9 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool differs(int k, const int *a, int n, int x)
{
  return !(0 <= k && k < n) || a[k] != x;
}

int search(const int *a, int n, int x)
    CERASTES_REQUIRES(cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && (r != -1 || cerastes_forall(differs, a, n, x)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(differs, a, i, x))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (a[i] == x) {
      return i;
    }
    i++;
  }
  return -1;
}
