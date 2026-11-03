// expect: error check @ L23
// mode: c23, c++20
// twin: tests/memory/struct_global_zero.c
// Pins: 0044 item 8: origin.x starts at 0, not 1: check at the start of main
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S origin;

void move(void)
    CERASTES_REQUIRES(cerastes_owns(origin) && origin.x < 100)
    CERASTES_ENSURES(cerastes_owns(origin) && origin.x == cerastes_old(origin.x) + 1 && origin.y == cerastes_old(origin.y))
{
  origin.x += 1;
}

int main(void)
{
  CERASTES_CHECK(origin.x == 1 && origin.y == 0);  // <- here
  move();
  CERASTES_CHECK(origin.x == 1 && origin.y == 0);
  origin.y = 5;
  move();
  CERASTES_CHECK(origin.x == 2 && origin.y == 5);
  return origin.x + origin.y;
}
