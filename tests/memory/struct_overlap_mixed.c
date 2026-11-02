// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: a reads atom on a struct beside an owns atom on an int of another object
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int mixed(struct S *s, int *q)
    CERASTES_REQUIRES(cerastes_reads(*s) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_reads(*s) && cerastes_owns(*q) && r == s->y)
{
  (void)q;
  return s->y;
}

int use_mixed(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct S s = {3, 4};
  struct S t = {5, 6};
  return mixed(&s, &t.x);
}
