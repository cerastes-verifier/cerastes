// expect: error out-of-bounds @ L18
// mode: c23, c++20
// twin: tests/memory/array_struct_access.c
// Pins: 0064 item 5: v[i].x with i == 3 reaches no element of a three-element array: out-of-bounds at the first access (the bound row precedes the cell's rows; a member step adds none), and nothing else is reported on the path
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i <= 3 && 0 <= j && j < 3 && i != j)
    CERASTES_ENSURES(r, r == 7)
{
  struct P v[3];
  v[i].x = 3;  // <- here
  v[i].y = 4;
  v[j].x = 5;
  v[j].y = 6;
  return v[i].x + v[i].y;
}
