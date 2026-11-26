// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/global_array_struct_values.c
// Pins: 0064 item 9: the list {{1, 2}, {.x = 3}} does not reach element 2, so g[2].y is zero by the quantifier of the rest, not 1
// Pre-registered from record 0064 item 9 (Part C2), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {.x = 3}};

int main(void)
{
  CERASTES_CHECK(g[2].y == 1);  // <- here
  return g[0].x - 1;
}
