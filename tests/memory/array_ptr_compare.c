// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 10: a == p compares the decayed a with p (p = &a[0]): equal; the null comparisons are the rejected ones
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

int same(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  int *p = &a[0];
  CERASTES_CHECK(a == p);
  return *p;
}
