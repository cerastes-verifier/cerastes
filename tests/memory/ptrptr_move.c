// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 7 (questions 104, 108): a callee that moves *pp to the object it was given returns it at the amount it was given: owns(**pp) after the move verifies; the caller keeps using y and has given x away
#include <cerastes.h>
#include <stddef.h>

void move(int **pp, int *q)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == q && cerastes_owns(**pp) && **pp == cerastes_old(*q))
{
  *pp = q;
}

int use_move(int *x)
    CERASTES_REQUIRES(cerastes_owns(*x))
    CERASTES_ENSURES(r, r == 5)
{
  int y = 5;
  int *p = x;
  move(&p, &y);
  CERASTES_CHECK(p == &y);
  return *p;
}
