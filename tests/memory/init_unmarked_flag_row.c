// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 items 7 and 9 (questions 82, 85): an unmarked owns or reads parameter is given an initialized &x
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 1)
{
  *p = 1;
}

int look(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int use_both(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  set(&x);
  return look(&x);
}
