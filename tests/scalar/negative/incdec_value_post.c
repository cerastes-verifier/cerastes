// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/incdec_value.c
// Pins: 0020 item 1: x++ is the old value: a + b is 2 * x + 2, not 2 * x + 3
#include <cerastes.h>

int both(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741822)
    CERASTES_ENSURES(r, r == 2 * x + 3)
{
  int a = x++;
  int b = ++x;
  return a + b;
}

int next(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x)
{
  int a = x++;
  return a;
}
