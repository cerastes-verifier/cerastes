// expect: error conversion @ L17
// mode: c23, c++20
// twin: tests/scalar/do_once.c
// Pins: 0014 item 5: the condition of a do loop is evaluated, with its conversion check
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
  } while ((signed char)256);
  return y;
}
