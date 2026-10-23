// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/comma_void_discard.c
// Pins: 0020 item 5: x++ stores x + 1
#include <cerastes.h>

int bump(int x, int y)
    CERASTES_REQUIRES(-1000 <= x && x <= 1000 && -1000 <= y && y <= 1000)
    CERASTES_ENSURES(r, r == x)
{
  (void)(x++, y);
  return x;
}
