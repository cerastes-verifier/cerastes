// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 2: x++ used as a value on a cell: the value is the snapshot taken before the write (y == 1), the cell holds the new value (re-expects the retired pin of 0032 items 9 and 10)
#include <cerastes.h>

int post_inc_value(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int *p = &x;
  int y = x++;
  CERASTES_CHECK(x == 2 && *p == 2);
  return y + *p;
}
