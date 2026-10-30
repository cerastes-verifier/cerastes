// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 (audit B3): q == NULL || (reads(*q) && *q == 0) with q null at entry (the amount snapshot is none) and set to the parameter in the loop
#include <cerastes.h>
#include <stddef.h>

int null_start(int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p == 0 && 0 <= n && n <= 10)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == 0)
{
  int *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (q == NULL || (cerastes_reads(*q) && *q == 0)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    i += 1;
  }
  return q == NULL ? 0 : *q;
}
