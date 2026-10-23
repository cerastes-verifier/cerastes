// expect: error precondition @ L17
// mode: c23, c++20
// twin: tests/scalar/call_cond_arm.c
// Pins: 0021: the condition a >= 0 lets a == 0 choose the call in the then arm
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int arm_then(int a)
    CERASTES_ENSURES(r, r == (a > 0 ? a : 0))
{
  return a >= 0 ? pos(a) : 0;
}

int arm_else(int a)
    CERASTES_ENSURES(r, r == (a <= 0 ? 0 : a))
{
  return a <= 0 ? 0 : pos(a);
}
