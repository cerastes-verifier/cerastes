// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: a pointer to a structure as an element: arr[0]->x = 1 loads the element, then stores through it
// Pre-registered from the M4.5e1a sweep (report 5, s08): verifies
#include <cerastes.h>

struct S {
  int x;
};

int through(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct S s;
  s.x = 0;
  struct S *arr[1] = {&s};
  arr[0]->x = 1;
  return s.x;
}
