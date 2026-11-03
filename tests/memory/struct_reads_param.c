// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 9: reads(*p) gives a fraction of every leaf and of the liveness cell, returned exactly: the caller calls twice and writes after
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int sum(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && 0 <= p->x && p->x <= 1000 && 0 <= p->y && p->y <= 1000)
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == p->x + p->y)
{
  return p->x + p->y;
}

int use_sum(void)
    CERASTES_ENSURES(r, r == 14)
{
  struct S s = {3, 4};
  int a = sum(&s);
  int b = sum(&s);
  s.x = 1;
  CERASTES_CHECK(s.x == 1 && s.y == 4);
  return a + b;
}
