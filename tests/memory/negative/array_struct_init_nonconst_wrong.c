// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/array_struct_init.c
// Pins: 0064 item 6: v[1].x is a + 1, so the check that it is a fails
// Pre-registered from record 0064 item 6 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int dynamic(int a, int b)
    CERASTES_REQUIRES(0 <= a && a < 100 && 0 <= b && b < 100)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{a, b}, {a + 1, 4}};
  CERASTES_CHECK(v[1].x == a);  // <- here
  return v[1].x - a - 1 + v[0].y - b;
}
