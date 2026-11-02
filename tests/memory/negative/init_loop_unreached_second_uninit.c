// expect: error uninitialized @ L29
// mode: c23, c++20
// twin: tests/memory/init_loop_unreached_second.c
// Pins: 0042 item 7 (audit round 2 p4): x has no value after the loop, so the call reports exactly one error, uninitialized at the call (the range row of an unmarked atom stays permission and is hidden behind the flag row)
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 1;
}

int f(int *q, int n)
    CERASTES_REQUIRES(cerastes_reads(*q) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_reads(*q))
{
  int x;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && 0 <= s && s <= 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*q > 0) {
      s = 1;
    }
    i += 1;
  }
  set(&x);  // <- here
  return s;
}
