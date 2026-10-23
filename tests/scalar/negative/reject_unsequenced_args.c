// expect: rejected cerastes-unsequenced_modification @ L17
// mode: c23, c++20
// twin: tests/scalar/incdec_argument.c
// Pins: 0020 item 5: the modification of x is unsequenced with another use of x in the same full expression
#include <cerastes.h>

int g(int a, int b)
    CERASTES_REQUIRES(0 <= a && a < 1000 && 0 <= b && b < 1000)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  return g(x++, x);
}
