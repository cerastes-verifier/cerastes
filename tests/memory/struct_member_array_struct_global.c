// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3 and 9 (Part C2): a structure global with an array member of structures and an initializer: program_start's row group has the member's leaf paths and the facts the member's nested lengths; read and written in main
// Pre-registered from record 0064 item 9 (Part C2), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

static struct T g = {{{1, 2}, {3, 4}}, 5};

int main(void)
{
  CERASTES_CHECK(g.v[0].x == 1 && g.v[1].y == 4 && g.n == 5);
  g.v[1].x = 7;
  CERASTES_CHECK(g.v[1].x == 7 && g.v[0].y == 2);
  return g.n - 5;
}
