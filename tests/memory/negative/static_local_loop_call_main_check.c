// expect: error check @ L26
// mode: c23, c++20
// twin: tests/memory/static_local_loop_call.c
// Pins: 0041 item 12 (audit round 2, SF1): main holds the static local from program_start but its value is unknown, and each call changes it: after three calls r == 3 is not known
#include <cerastes.h>

int f(void)
{
  static int c = 0;
  if (c < 100) {
    c++;
  }
  return c;
}

int main(void)
{
  int r = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    r = f();
    i++;
  }
  CERASTES_CHECK(r == 3);
  return r;
}
