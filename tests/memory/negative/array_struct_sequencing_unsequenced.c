// expect: error unsequenced @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_sequencing.c
// Pins: 0064 item 11 (0039 item 3): without i != j the write to v[i].x and the increment of v[j].x may reach one leaf of one element (the path-precise row)
// Pre-registered from record 0064 item 11 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int elements(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  v[i].x = v[j].x++;  // <- here
  return v[0].y;
}
