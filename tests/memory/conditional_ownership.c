// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 7 and 0037 item 5: c ? owns(*p) : reads(*p) in requires and ensures: written under c, read otherwise; the callers pass 1, an unknown c and 0
#include <cerastes.h>
#include <stddef.h>

int step(int c, int *p)
    CERASTES_REQUIRES((c ? cerastes_owns(*p) : cerastes_reads(*p)) && *p >= 0 && *p < 100)
    CERASTES_ENSURES(r, (c ? cerastes_owns(*p) : cerastes_reads(*p)) && r == *p && r == cerastes_old(*p) + (c != 0 ? 1 : 0))
{
  if (c) {
    *p += 1;
  }
  return *p;
}

int use_step(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 8 : 6))
{
  int x = 1;
  int a = step(1, &x);
  int b = step(c, &x);
  int d = step(0, &x);
  return a + b + d;
}
