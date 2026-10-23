// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/discarded_and_assign_const.c
// Pins: 0020 item 5 (audit round 2): the assignment runs when a is nonzero: y is 2 then, not 3
#include <cerastes.h>

int cond_two(int a, int y)
    CERASTES_ENSURES(r, r == (a != 0 ? 3 : y))
{
  a ? (y = 2) : 0;
  return y;
}

int cond_zero(int a, int y)
    CERASTES_ENSURES(r, r == (a != 0 ? 0 : y))
{
  a ? (y = 0) : 0;
  return y;
}
