// expect: error check @ L19
// mode: c23, c++20
// twin: tests/memory/global_loop.c
// Pins: 0041 item 10: h <= 7 + i keeps h from overflowing but does not say what h is after the loop: h == 17 fails (g, never written, is still 3)
#include <cerastes.h>

static int g = 3;
static int h = 7;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && h <= 7 + i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    h += 1;
    i += 1;
  }
  CERASTES_CHECK(g == 3 && h == 17);
  return g - g;
}
