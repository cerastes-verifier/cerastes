// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 and 0038 item 3: a loop that only reads through a pointer cell keeps the cell's value (the automatic invariant, with WF in place of a range)
#include <cerastes.h>
#include <stddef.h>

int loop_read(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7 * n)
{
  int x = 7;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 7 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += **pp;
    i += 1;
  }
  CERASTES_CHECK(p == &x);
  return s;
}
