// expect: error check @ L21
// mode: c23, c++20
// twin: tests/memory/array_struct_copy.c
// Pins: 0064 item 12: v[i] = v[j] copies {9, 8}, so the check that v[i].y is 7 fails (two elements: the three-element form took 125 s, question 328)
// Pre-registered from record 0064 item 12 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int to_elem(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2)
    CERASTES_ENSURES(r, r == 0)
{
  struct P v[2] = {{1, 2}, {3, 4}};
  struct P t = {9, 8};
  v[j] = t;
  v[i] = v[j];
  CERASTES_CHECK(v[i].y == 7);  // <- here
  return v[i].x - 9 + v[j].y - 8;
}
