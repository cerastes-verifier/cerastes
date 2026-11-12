// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 11: the values of a range are kept across a read-only loop (the values-kept conjunct of a read-only place): a check after the loop still reads p[0] == 7
// Pre-registered from record 0060 item 11 (Part D), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

int count_sevens(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0 && p[0] == 7)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && 0 <= r && r <= n)
{
  int i = 0;
  int hits = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= hits && hits <= i) CERASTES_DECREASES(n - i) while (i < n) {
    if (p[i] == 7) {
      hits++;
    }
    i++;
  }
  CERASTES_CHECK(p[0] == 7);
  return hits;
}
