// expect: error check @ L16
// mode: c23, c++20
// twin: tests/scalar/do_once_decreases.c
// Pins: 0014 item 5: the body of a do ... while (0) with a written decreases is verified
// No error twin can break the measure itself: a loop that never iterates twice has no measure to compare.
#include <cerastes.h>

int add_one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x;
  CERASTES_DECREASES(1)
  do {
    y += 1;
    CERASTES_CHECK(y == x);
  } while (0);
  return y;
}
