// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_init.c
// Pins: 0064 item 5: the array has three elements, so sizeof(v) / sizeof(v[0]) == 2 fails
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int size(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  CERASTES_CHECK(sizeof(v) / sizeof(v[0]) == 2);  // <- here
  return (int)(sizeof(v) / sizeof(v[0])) + v[0].x - 1;
}
