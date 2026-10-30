// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 3: x++ + *p with p the address of another cell: the obligation x != p holds (path-precise); x++ is a snapshot, the read of *p follows the write of the field
#include <cerastes.h>

int sum_after(void)
    CERASTES_ENSURES(r, r == 13)
{
  int x = 1;
  int y = 10;
  int *px = &x;
  int *p = &y;
  int s = x++ + *p;
  return s + *px;
}
