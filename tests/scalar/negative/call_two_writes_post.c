// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/scalar/call_two_writes.c
// Pins: 0025 item 2: the values of the stores are the callees' results, so a postcondition off by one fails
#include <cerastes.h>

int capped(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == (x > 1000 ? 1000 : x));

int small(int y)
    CERASTES_REQUIRES(y >= 0)
    CERASTES_ENSURES(r, r == (y > 500 ? 500 : y));

int two_writes(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == 2 * ((a > 1000 ? 1000 : a) + (b > 500 ? 500 : b)) + 1)
{
  int x;
  int y;
  int s = (x = capped(a)) + (y = small(b));
  return s + x + y;
}
