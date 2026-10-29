// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 4: a callee that owns the cell and gives it back (inc) called in a loop; the user's invariant carries the value, the iteration check finds the permission
#include <cerastes.h>

int inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p);

int bump_n(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == n)
{
  int x = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int v = inc(&x);
    i = v;
  }
  return x;
}
