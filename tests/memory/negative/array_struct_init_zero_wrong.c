// expect: error check @ L17
// c-only: the twin of a C-only test
// twin: tests/memory/array_struct_init_zero.c
// Pins: 0064 item 16: {0} over an array of structures zeroes every leaf of every element, so the check that v[1].y is 1 fails
// Pre-registered from record 0064 item 16 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int locals(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {0};
  CERASTES_CHECK(v[1].y == 1);  // <- here
  return v[0].x;
}
