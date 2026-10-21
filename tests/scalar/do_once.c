// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 5 (owner, 2026-10-20): do ... while (0) needs no decreases; a break leaves it
// The macro idiom: a loop that runs once, left early by break. No measure is written.
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
  } while (0);
  return y;
}
