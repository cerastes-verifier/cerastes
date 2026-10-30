// expect: error termination @ L15
// mode: c23, c++20
// twin: tests/memory/global_old_decreases.c
// Pins: 0041 item 7 (0039 item 8): decreases(g) with no decrement before the recursive call: termination at the call
#include <cerastes.h>

static int g = 5;

void count_down(void)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= g && g <= 100)
    CERASTES_ENSURES(cerastes_owns(g))
    CERASTES_DECREASES(g)
{
  if (g > 0) {
    count_down();
  }
}
