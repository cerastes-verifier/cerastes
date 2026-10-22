// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_multi_declarator.c
// Pins: 0019 item 1: both declarators are defined by the time they are read; the sum is wrong
#include <cerastes.h>

int two(int a)
    CERASTES_REQUIRES(0 <= a && a <= 100)
    CERASTES_ENSURES(r, r == a + 3)
{
  int x, y = 1;
  x = a;
  y += 1;
  return x + y;
}
