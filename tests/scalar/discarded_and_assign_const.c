// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 5 (audit round 2): a discarded ?: that assigns a constant in an arm is accepted, the zero constant included. The same assignment as the right operand of a discarded && cannot be a verified test: EDG prints its #187 for a constant assigned in a truth context (accepted for a nonzero constant or a bool target, rejected as value_not_used for zero to an int)
#include <cerastes.h>

int cond_two(int a, int y)
    CERASTES_ENSURES(r, r == (a != 0 ? 2 : y))
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
