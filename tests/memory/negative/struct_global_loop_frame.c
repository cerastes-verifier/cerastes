// expect: error invariant-preserved @ L18
// mode: c23, c++20
// twin: tests/memory/struct_global_loop.c
// Pins: 0044 item 8: the loop writes g.x, so claiming g.x == 0 in the invariant is not preserved
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g = {0, 5};
static int h = 7;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && g.x == 0)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  return i + (h - 7) + (g.y - 5);
}
