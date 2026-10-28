// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 and 0032 item 2: a pointer local without an initializer is assigned on every path before it is read
#include <cerastes.h>

int select_cell(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 2))
{
  int x = 1;
  int y = 2;
  int *p;
  if (c != 0) {
    p = &x;
  } else {
    p = &y;
  }
  return *p;
}
