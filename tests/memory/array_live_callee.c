// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 6 (0037 item 13): a callee that needs the liveness of &a[1] and gives it back: the scope exit's unmarked liveness row holds
// Pre-registered from audit round 2 of record 0059 (item 5a): verifies
#include <cerastes.h>

void hold(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(cerastes_live(p));

int f(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[2] = {1, 2};
  hold(&a[1]);
  return a[0];
}
