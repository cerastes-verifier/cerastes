// expect: error invariant-preserved @ L17
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_pick.c
// Pins: 0040 item 2 (audit B1): *pp == &x alone is not preserved: the loop may store &y
#include <cerastes.h>
#include <stddef.h>

int pick_cell(int c, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
    CERASTES_ENSURES(r, r == 1 || r == 2)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *pp == &x)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pp = c ? &x : &y;
    i += 1;
  }
  return **pp;
}
