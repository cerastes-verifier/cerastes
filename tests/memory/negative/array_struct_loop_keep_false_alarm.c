// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/array_struct_loop.c
// Pins: 0064 item 10 (listed false alarm, question 318): the loop starts at i == 1 and writes v[i].x, so v[0].x is never written and the check that it is still 0 is true; but a store to v[i].x reaches every place of the field x, so the automatic conjuncts keep no value of x and the check fails (a pinned false alarm; a written quantified invariant is Part F's and M4.8's)
// Pre-registered from record 0064 item 10 (Part B), before any run: the one error (false alarm, fail closed)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int tail(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct P v[3] = {{0, 2}, {0, 4}, {0, 6}};
  int i = 1;
  CERASTES_INVARIANT(1 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    v[i].x = 5;
    i = i + 1;
  }
  CERASTES_CHECK(v[0].x == 0);  // <- here
  return i + v[0].y * 0;
}
