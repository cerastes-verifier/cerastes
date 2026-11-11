// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: an int ** element: **pp[0] = 1 loads the element, then the pointer it points to, then stores
// Pre-registered from the M4.5e1a sweep (report 5, s09): verifies
#include <cerastes.h>

int through(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  int *p = &x;
  int **pp[1] = {&p};
  **pp[0] = 1;
  return x;
}
