// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 6: a pointer the loop assigns (p = c ? &x : &y), used before the reassignment and after the loop, with p == &x || p == &y in the invariant (case splits at the top of the body and after the loop)
#include <cerastes.h>

int pick_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1 || r == 2)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == &x || p == &y))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*p == 1) {
      i += 1;
    } else {
      i += 1;
    }
    p = (i % 2 == 0) ? &y : &x;
  }
  return *p;
}
