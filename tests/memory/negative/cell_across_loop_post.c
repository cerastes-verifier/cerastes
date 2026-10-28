// expect: error postcondition @ L22
// mode: c23, c++20
// twin: tests/memory/cell_across_loop.c
// Pins: audit round 2 of M4.5b1a, 0032 item 4: the cell still holds 7 after the loop, not 8
#include <cerastes.h>

int across(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 8)
{
  int x = 7;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 50) {
      break;
    }
    i += 1;
  }
  return *p;
}
