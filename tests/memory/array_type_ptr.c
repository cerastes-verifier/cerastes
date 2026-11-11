// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: a pointer element (v_ptr_int): *ps[0] is a load of the element then a store through it
// Pre-registered from record 0059 item 2: verifies
#include <cerastes.h>

int through(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *ps[2] = {&x, &x};
  *ps[0] = 1;
  return x;
}
