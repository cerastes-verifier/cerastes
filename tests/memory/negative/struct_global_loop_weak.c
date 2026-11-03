// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/struct_global_loop.c
// Pins: 0044 item 8: without the clause (i == 0 || g.x == 1) g.x is known after the loop only by its range: the check fails
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
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  CERASTES_CHECK(g.x == 1 && g.y == 5 && h == 7);  // <- here
  return h + g.y;
}
