// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/or_assign.c
// Pins: 0020 item 3: for 1 <= a <= 5 the result is a + 1
#include <cerastes.h>

int mid(int a)
    CERASTES_ENSURES(r, r == ((0 < a && a <= 5) ? a + 1 : 0))
{
  int x;
  if (a <= 0 || (x = a) > 5) {
    return 0;
  }
  return x + 2;
}
