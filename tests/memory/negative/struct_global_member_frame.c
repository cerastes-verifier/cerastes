// expect: error check @ L36
// mode: c23, c++20
// twin: tests/memory/struct_global_member.c
// Pins: 0044 item 8: bump() holds only g.x, so g.y is kept: check(g.y == 1) fails
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S g = {5, 0};

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g.x) && g.x < 100)
    CERASTES_ENSURES(cerastes_owns(g.x) && g.x == cerastes_old(g.x) + 1)
{
  g.x += 1;
}

void down(void)
    CERASTES_REQUIRES(cerastes_owns(g.x) && 0 <= g.x && g.x <= 10)
    CERASTES_ENSURES(cerastes_owns(g.x) && g.x == 0)
    CERASTES_DECREASES(g.x)
{
  if (g.x > 0) {
    g.x -= 1;
    down();
  }
}

int main(void)
{
  CERASTES_CHECK(g.x == 5);
  bump();
  CERASTES_CHECK(g.x == 6 && g.y == 1);  // <- here
  down();
  CERASTES_CHECK(g.x == 0 && g.y == 0);
  return g.x + g.y;
}
