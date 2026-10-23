// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/comma_statement.c
// Pins: 0020 item 2: x = a, y = b stores both
#include <cerastes.h>

int pair(int a, int b)
    CERASTES_REQUIRES(0 <= b && a <= 2147483647 - b)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int y;
  x = a, y = b;
  return x + y;
}
