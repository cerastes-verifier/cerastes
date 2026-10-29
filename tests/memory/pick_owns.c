// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0037 items 3, 6, 8: a pointer result related to the parameters (r == p || r == q); *pick(...) verifies by a case split on the result; an entry r == p || r == q lets *r = 1 write one of the two cells
#include <cerastes.h>
#include <stddef.h>

int *pick(int c, int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(r, (r == p || r == q) && cerastes_owns(*p) && cerastes_owns(*q))
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
