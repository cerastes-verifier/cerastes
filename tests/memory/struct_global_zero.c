// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 8: a static struct global without an initializer is zero in every member, known to main (internal linkage, both languages); owns(origin) is every member and one liveness fraction, old(origin.x) and the frame of origin.y through the call
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
  CERASTES_CHECK(origin.x == 0 && origin.y == 0);
  move();
  CERASTES_CHECK(origin.x == 1 && origin.y == 0);
  origin.y = 5;
  move();
  CERASTES_CHECK(origin.x == 2 && origin.y == 5);
  return origin.x + origin.y;
}
