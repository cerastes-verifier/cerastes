// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/global_array_struct_values.c
// Pins: 0064 item 9: the element {.x = 3} leaves its member y out, so g[1].y is zero (by the list, per leaf), not 1
// Pre-registered from record 0064 item 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {.x = 3}};

int main(void)
{
  CERASTES_CHECK(g[1].y == 1);  // <- here
  return g[0].x - 1;
}
