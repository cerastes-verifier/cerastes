// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 4 (audit round 2 S4): a reads(*p) place returned by the ensures keeps its entry value, so *r > 0 and 10 / *r is defined
#include <cerastes.h>
#include <stddef.h>

int *pick(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p > 0)
    CERASTES_ENSURES(r, r == p && cerastes_reads(*p) && 10 / *r >= 0)
{
  return p;
}

int use(void)
{
  int x = 3;
  return *pick(&x);
}
