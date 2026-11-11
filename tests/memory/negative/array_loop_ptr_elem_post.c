// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/memory/array_loop_ptr_elem.c
// Pins: 0059 item 8: two iterations, so r == 3 fails
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int count(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 0;
  int y = 0;
  int *ps[2] = {&x, &y};
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(c == i)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    *ps[i] = 1;
    c = c + 1;
    i = i + 1;
  }
  return c;
}
