// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11: p[0] in an invariant and in a measure (countdown), and p[i - 1] under i == 0 || in an invariant (first_negative): the reads are framed by the loop's range place (the interval test of item 5)
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void countdown(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && 0 <= p[0] && p[0] <= 100)
    CERASTES_ENSURES(cerastes_owns_range(p, n) && p[0] == 0)
{
  (void)n;
  CERASTES_INVARIANT(0 <= p[0] && p[0] <= 100) CERASTES_DECREASES(p[0]) while (p[0] > 0) {
    p[0] -= 1;
  }
}

int first_negative(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n))
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && (r == -1 || (0 <= r && r < n && p[r] < 0)))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || p[i - 1] >= 0)) CERASTES_DECREASES(n - i) while (i < n) {
    if (p[i] < 0) {
      return i;
    }
    i++;
  }
  return -1;
}
