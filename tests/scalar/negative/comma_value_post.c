// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/comma_value.c
// Pins: 0020 item 2: the comma's value is x + b, after x = a
#include <cerastes.h>

int seq(int a, int b)
    CERASTES_REQUIRES(0 <= b && a <= 2147483647 - b)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int y = (x = a, x + b);
  return y;
}
