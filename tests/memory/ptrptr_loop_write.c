// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2 and 0038 items 3, 6: a write through a T** in a loop havocs the pointer cell; the invariant says where it points (case splits against the automatic places), so a check on *p after the loop holds
#include <cerastes.h>
#include <stddef.h>

int loop_write(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 15)
{
  int x = 7;
  int y = 8;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == &x || p == &y))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pp = &y;
    i += 1;
  }
  CERASTES_CHECK(*p == 7 || *p == 8);
  return x + y;
}
