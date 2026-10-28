// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 and 0032 item 6: a cell is initialized by a write through a pointer to it, then read by name
#include <cerastes.h>

int through(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int *p = &x;
  *p = a;
  return x;
}
