// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 items 3 and 8 (audit B1, with a call): a bodiless contract whose requires and ensures carry the same guard n == 0 ||: called with (a, 0), where the guard is false and the array and its values stay the caller's (the check), and with (a, 2)
// Pre-registered from record 0060 items 3 and 8 (Part C), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void g(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(n == 0 || cerastes_owns_range(p, n));

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[2] = {1, 2};
  g(a, 0);
  CERASTES_CHECK(a[0] == 1);
  g(a, 2);
  return 1;
}
