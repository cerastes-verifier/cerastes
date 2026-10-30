// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 (audit B1): a loop that stores *pp = c ? &x : &y, the invariant saying where *pp points, then **pp after the loop (case splits for loaded pointers)
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
  CERASTES_INVARIANT(0 <= i && i <= n && (*pp == &x || *pp == &y))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pp = c ? &x : &y;
    i += 1;
  }
  return **pp;
}
