// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/memory/pick_owns.c
// Pins: 0037 item 10: the result may be q, so r == p fails at the return
#include <cerastes.h>
#include <stddef.h>

int *pick(int c, int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, r == p && cerastes_owns(*p) && cerastes_owns(*q))
{
  return c ? p : q;
}

int use_pick(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 1;
  int y = 2;
  int *t = pick(1, &x, &y);
  *t = 7;
  return *t;
}

void set_one(int *p, int *q, int *r)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && (r == p || r == q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q))
{
  (void)p;
  (void)q;
  *r = 1;
}
