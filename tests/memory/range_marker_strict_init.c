// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// Pins: 0060 item 12 (0042 item 8): under --strict-init an uninit array local is an accepted argument of a must_init range parameter, as &x of an uninit local is of a scalar one
// Pre-registered from record 0060 item 12 (Part E), before the code: verifies
#include <cerastes.h>
#include <stddef.h>

void fill3(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3))
    CERASTES_ENSURES(cerastes_owns_range(p, 3) && p[0] == 1)
{
  p[0] = 1;
  p[1] = 2;
  p[2] = 3;
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  CERASTES_UNINIT int a[3];
  fill3(a);
  return a[0] + (a[2] - a[2]);
}
