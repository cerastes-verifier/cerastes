// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0060 item 15 (Step 0 M: no compiler warns): `int a[]` against `int *a` is accepted: the same pointer parameter
// Pre-registered from record 0060 item 15 (Part E), before the code: verifies
#include <cerastes.h>

int f(int a[]);

int f(int *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a))
{
  return a[0];
}

int use(void)
{
  int x[2] = {5, 6};
  return f(x);
}
