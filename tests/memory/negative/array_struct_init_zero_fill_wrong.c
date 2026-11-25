// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_init.c
// Pins: 0064 item 6: the partial list {{1, 2}} leaves b[2] to the zero fill, so the check that b[2].y is 1 fails
// Pre-registered from record 0064 item 6 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int lists(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P b[3] = {{1, 2}};
  CERASTES_CHECK(b[2].y == 1);  // <- here
  return b[0].x - 1;
}
