// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 15: owns_range(a, 3) on `int a[3]`: the written bound is the count the contract gives (the language ignores it): the body reads a[0], the caller passes its local array whole
// Pre-registered from record 0060 item 15 (Part E), before the code: verifies
#include <cerastes.h>

int first(int a[3])
    CERASTES_REQUIRES(cerastes_owns_range(a, 3))
    CERASTES_ENSURES(r, cerastes_owns_range(a, 3) && r == a[0] && a[0] == cerastes_old(a[0]))
{
  return a[0];
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x[3] = {7, 8, 9};
  return first(x);
}
