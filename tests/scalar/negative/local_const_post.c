// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/local_const.c
// Pins: 0010 item 12: x * k reaches 1000, so r < 1000 is false
#include <cerastes.h>

int scaled(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 10)
    CERASTES_ENSURES(r, r < 1000)
{
  const int k = 100;
  return x * k;
}
