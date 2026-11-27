// expect: error invariant-preserved @ L18
// mode: c23, c++20
// twin: tests/memory/array_struct_loop.c
// Pins: 0064 item 10 (the positive's member_inv): the invariant reads the member v[1].x of an element of a local array; it holds at entry (the initializer), but the loop writes v[1].x = 9, so the conjunct is not preserved at the back edge (a true failure, the one error; the s bound is left out so that nothing else fails)
// Pre-registered from record 0064 item 10 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

int member_inv(void)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(v[1].x == 3)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    v[1].x = 9;
    i = i + 1;
  }
  return i + v[0].y * 0;
}
