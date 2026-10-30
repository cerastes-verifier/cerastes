// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/memory/loop_reads_alias.c
// Pins: 0038 item 3: without the upper bound on *c the sum may overflow (the invariant and the ensures state no upper bound, so the overflow is the one failure)
#include <cerastes.h>

int sum3(const int *a, const int *b, const int *c, int n)
    CERASTES_REQUIRES(cerastes_reads(*a) && cerastes_reads(*b) && cerastes_reads(*c) && 0 <= *a && *a <= 10 && 0 <= *b && *b <= 10 && 0 <= *c && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads(*a) && cerastes_reads(*b) && cerastes_reads(*c) && 0 <= r)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *a + *b + *c;
    i += 1;
  }
  return s;
}
