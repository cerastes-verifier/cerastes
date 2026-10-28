// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 4, 6: two pointers to one cell: a write through one is seen through the other and by name
#include <cerastes.h>

int alias(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int *p = &x;
  int *q = p;
  *q = 3;
  CERASTES_CHECK(x == 3);
  return *p;
}
