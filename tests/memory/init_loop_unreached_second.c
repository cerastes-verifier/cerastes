// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 5, 7 (audit round 2 p4): after a loop that only reads *q, an initialized x passed to an unmarked owns parameter is fine
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
  int x = 0;
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
  set(&x);
  return s;
}
