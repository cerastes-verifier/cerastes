// expect: error termination @ L27
// mode: c23, c++20
// twin: tests/memory/struct_global_member.c
// Pins: 0044 item 10: g.x is not decremented before the recursive call: termination at the call
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
    down();  // <- here
  }
}

int main(void)
{
  CERASTES_CHECK(g.x == 5);
  bump();
  CERASTES_CHECK(g.x == 6 && g.y == 0);
  down();
  CERASTES_CHECK(g.x == 0 && g.y == 0);
  return g.x + g.y;
}
