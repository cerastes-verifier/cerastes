// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 items 1-2: a local declared without an initializer may be read once it is assigned
#include <cerastes.h>

int copy_first(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  int y;
  x = a;
  y = x;
  return y;
}
