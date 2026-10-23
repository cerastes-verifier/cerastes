// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/and_assign.c
// Pins: 0020 item 3: for a outside 1 to 4 the result is 0
#include <cerastes.h>

int small(int a)
    CERASTES_ENSURES(r, r == ((0 < a && a < 5) ? a + 1 : 0))
{
  int x;
  if (a > 0 && (x = a) < 5) {
    return x + 1;
  }
  return 1;
}
