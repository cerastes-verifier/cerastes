// expect: error check @ L18
// mode: c23, c++20
// twin: tests/scalar/do_once.c
// Pins: 0014 item 5: the body of a do ... while (0) is verified
// y == x + 1 after the increment, so the check fails; the loop itself needs no measure.
#include <cerastes.h>

int clamp_step(int x)
    CERASTES_ENSURES(r, (x > 50 && r == 60) || (x <= 50 && r == x + 1))
{
  int y = x;
  do {
    if (x > 50) {
      y = 60;
      break;
    }
    y += 1;
    CERASTES_CHECK(y == x);
  } while (0);
  return y;
}
