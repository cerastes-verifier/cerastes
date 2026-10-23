// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/scalar/sequenced_operands.c
// Pins: 0020 item 4: x++ in the condition stores x + 1
#include <cerastes.h>

int store_and(int a)
    CERASTES_ENSURES(r, r == (a > 5 ? a : 0))
{
  int x;
  if ((x = a) && x > 5) {
    return x;
  }
  return 0;
}

int incr_and(int x)
    CERASTES_REQUIRES(x <= 2147483546)
    CERASTES_ENSURES(r, r == (x != 0 && x >= -5 ? x + 101 : -(x + 1)))
{
  if (x++ && x > -5) {
    return x + 100;
  }
  return x;
}
