// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (from reject_call_cond.c): a call in an arm of ?: runs only when that arm is chosen; its precondition holds there
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int arm_then(int a)
    CERASTES_ENSURES(r, r == (a > 0 ? a : 0))
{
  return a > 0 ? pos(a) : 0;
}

int arm_else(int a)
    CERASTES_ENSURES(r, r == (a <= 0 ? 0 : a))
{
  return a <= 0 ? 0 : pos(a);
}
