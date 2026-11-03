// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 8 (0041 item 10, 0043 item 11): every leaf of a struct global and the scalar global h are automatic places of the loop in main; g.y and h keep their values, g.x is pinned by the invariant
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
  CERASTES_INVARIANT(0 <= i && i <= 3 && (i == 0 || g.x == 1))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g.x = 1;
    i += 1;
  }
  CERASTES_CHECK(g.x == 1 && g.y == 5 && h == 7);
  return h + g.y;
}
