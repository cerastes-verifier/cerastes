// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 5: reads(*pp) && reads(**pp) on both sides: a reads place is not written, so the callee's result and the caller's x agree
#include <cerastes.h>
#include <stddef.h>

int peek2(int **pp)
    CERASTES_REQUIRES(cerastes_reads(*pp) && cerastes_reads(**pp) && **pp < 100)
    CERASTES_ENSURES(r, cerastes_reads(*pp) && cerastes_reads(**pp) && r == **pp + 1)
{
  return **pp + 1;
}

int use(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 5;
  int *p = &x;
  int v = peek2(&p);
  CERASTES_CHECK(x == 5);
  return v;
}
