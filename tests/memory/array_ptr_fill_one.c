// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 10 and 0037: f(a) passes &a[0]: a callee that owns *p takes the first element and gives it back
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

void fill_one(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5)
{
  *p = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int a[3] = {1, 2, 3};
  fill_one(a);
  return a[0];
}
