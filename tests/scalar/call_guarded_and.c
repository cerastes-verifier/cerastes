// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (from reject_call_guarded.c): a call in the right operand of && runs only when the left operand is true; its precondition holds there
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int guarded(int a)
    CERASTES_ENSURES(r, r == (a > 1))
{
  return a > 0 && pos(a) > 1;
}
