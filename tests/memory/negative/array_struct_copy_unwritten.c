// expect: error uninitialized @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_copy.c
// Pins: 0064 item 12: v[1].x was written and v[1].y was not, so the copy t = v[1] reads one unwritten source leaf: uninitialized at the copy (one leaf, so one row)
// Pre-registered from record 0064 item 12 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int from_elem(void)
{
  struct P v[2];
  v[1].x = 1;
  struct P t = v[1];  // <- here
  return t.x;
}
