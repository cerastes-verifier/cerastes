// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/return_incdec.c
// Pins: 0020 item 1: ++x is the new value, so r == x + 1
#include <cerastes.h>

int post(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x)
{
  return x++;
}

int pre(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x)
{
  return ++x;
}
