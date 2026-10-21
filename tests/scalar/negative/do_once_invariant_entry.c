// expect: error invariant-entry @ L11
// mode: c23, c++20
// twin: tests/scalar/do_once.c
// Pins: 0014 item 5: a written invariant of a do ... while (0) is checked on entry
#include <cerastes.h>

int clamp_step(int x)
    CERASTES_ENSURES(r, (x > 50 && r == 60) || (x <= 50 && r == x + 1))
{
  int y = x;
  CERASTES_INVARIANT(y != x)
  do {
    if (x > 50) {
      y = 60;
      break;
    }
    y += 1;
  } while (0);
  return y;
}
