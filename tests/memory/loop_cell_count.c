// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 2 and 3: a cell written by name in a loop, its value in the invariant, read through a pointer after it (the shape that b1a rejected)
#include <cerastes.h>

int count_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    x += 1;
    i += 1;
  }
  return *p;
}
