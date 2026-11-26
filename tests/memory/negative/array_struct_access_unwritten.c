// expect: error uninitialized @ L18
// mode: c23, c++20
// twin: tests/memory/array_struct_access.c
// Pins: 0064 item 5 (0019, path-precise): v[i].x was written and v[i].y was not, so the read of v[i].y is uninitialized (the flag of that leaf of the element at i)
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int rw(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
{
  struct P v[3];
  v[i].x = 3;
  return v[i].y;  // <- here
}
