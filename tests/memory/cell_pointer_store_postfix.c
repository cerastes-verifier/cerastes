// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 3: *p = (*q)++ with p and q the addresses of different cells: the path-precise unsequenced obligation p != q holds; the value stored is the snapshot taken before q's write
#include <cerastes.h>

int copy_inc(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  int y = 5;
  int *p = &x;
  int *q = &y;
  *p = (*q)++;
  CERASTES_CHECK(x == 5 && y == 6);
  return x;
}
