// expect: error overflow @ L16
// mode: c23, c++20
// twin: tests/memory/loop_reads_sum.c
// Pins: 0038 item 3: without the upper bound on *p the sum may overflow (the invariant and the ensures state no upper bound, so the overflow is the one failure)
#include <cerastes.h>

int sum_reads(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= *p && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_reads(*p) && 0 <= r)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += *p;
    i += 1;
  }
  return s;
}
