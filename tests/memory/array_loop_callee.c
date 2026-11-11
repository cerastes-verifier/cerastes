// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a callee that owns one element and gives it back, in the loop body: the back-edge rows hold
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5)
{
  *p = 5;
}

int run(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    set(&a[1]);
    i = i + 1;
  }
  return i;
}
