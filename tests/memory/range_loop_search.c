// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11 (S44 (g) search): a loop over reads_range with a return in its body: the return rows run inside the body, the borrowed range is given back at every exit, the ensures reads p[r] under r != -1 (framed by the range) and the values are kept (a read-only place); no quantifier in the invariant
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int search(const int *p, int n, int x)
    CERASTES_REQUIRES(cerastes_reads_range(p, n))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == -1 || (0 <= r && r < n && p[r] == x)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    if (p[i] == x) {
      return i;
    }
    i++;
  }
  return -1;
}
