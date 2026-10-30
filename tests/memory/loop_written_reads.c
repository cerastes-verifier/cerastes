// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5: reads(*p) written in an invariant for a reassigned parameter; the atom frames the invariant's reads of *p
#include <cerastes.h>
#include <stddef.h>

int total(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= *p && *p <= 10 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads(*p) && 0 <= r && r <= 10 * n)
{
  int *q = p;
  p = q;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 10 * i && cerastes_reads(*p) && 0 <= *p && *p <= 10)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *p;
    i += 1;
  }
  return s;
}
