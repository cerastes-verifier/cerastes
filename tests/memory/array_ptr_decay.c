// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 10: int *p = a; is the address of the first element: p[2] = 7 writes a[2], which the array sees
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

int decay(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {1, 2, 3};
  int *p = a;
  p[2] = 7;
  CERASTES_CHECK(a[2] == 7);
  return a[2];
}
