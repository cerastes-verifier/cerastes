// expect: error uninitialized @ L30
// mode: c23, c++20
// twin: tests/memory/init_loop_unreached.c
// Pins: 0042 item 5 (audit S1): neither the loop nor leave gives x a value: the read after the call is reported
#include <cerastes.h>

void leave(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

int after_loop(int *q, int n)
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
  leave(&x);
  return s > 5 ? 0 : x;  // <- here
}
