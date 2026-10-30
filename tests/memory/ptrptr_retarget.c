// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 2: *pp = &y stores a pointer into the cell p; reading *p then reads y
#include <cerastes.h>
#include <stddef.h>

int retarget(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 1;
  int y = 7;
  int *p = &x;
  int **pp = &p;
  *pp = &y;
  CERASTES_CHECK(p == &y);
  return *p;
}
