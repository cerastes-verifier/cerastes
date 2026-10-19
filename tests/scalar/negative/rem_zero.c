// expect: error division-by-zero @ L17
// mode: c23, c++20
// twin: tests/scalar/div_trunc.c
// Pins: 0010 item 7: a zero divisor of % is a division by zero
#include <cerastes.h>

int quot(int a, int b)
    CERASTES_REQUIRES(a == -7 && b == 2)
    CERASTES_ENSURES(r, r == -3)
{
  return a / b;
}

int rem(int a, int b)
    CERASTES_REQUIRES(a == -7 && b >= 0)
{
  return a % b;
}
