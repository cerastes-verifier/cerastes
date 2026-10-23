// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: a call in the right operand of || runs only when the left operand is false; its precondition holds there
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int guarded_or(int a)
    CERASTES_ENSURES(r, r == (a != 1))
{
  return a <= 0 || pos(a) > 1;
}
