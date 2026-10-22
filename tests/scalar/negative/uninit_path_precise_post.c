// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_path_precise.c
// Pins: 0019 item 3: the guarded read verifies but the returned value is wrong
#include <cerastes.h>

int guarded(int c)
    CERASTES_ENSURES(r, (c != 0 && r == 1) || (c == 0 && r == 0))
{
  int x;
  if (c != 0) {
    x = 2;
  }
  if (c != 0) {
    return x;
  }
  return 0;
}
