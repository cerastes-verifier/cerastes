// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/early_return.c
// Pins: 0012 item 9: the middle return is out of the range
#include <cerastes.h>

int sign(int x)
    CERASTES_ENSURES(r, (x < 0 && r == -1) || (x == 0 && r == 0) || (x > 0 && r == 1))
{
  if (x < 0) {
    return -1;
  }
  if (x > 0) {
    return 2;
  }
  return 0;
}
