// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_assign_read.c
// Pins: 0019 item 2: the assignment gives x a value, but the wrong one
#include <cerastes.h>

int copy_first(int a)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int y;
  x = a + 1;
  y = x;
  return y;
}
