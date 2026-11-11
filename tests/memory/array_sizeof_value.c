// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 11: sizeof(a) of an array local is N * sizeof(T), computed from the checked bound: 3 * 4 = 12
// Pre-registered from record 0059 item 11 (Part E), before the code: verifies
#include <cerastes.h>

int size(void)
    CERASTES_ENSURES(r, r == 12)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(sizeof(a) == 3 * sizeof(int));
  return (int)sizeof(a) + a[0] - 1;
}
