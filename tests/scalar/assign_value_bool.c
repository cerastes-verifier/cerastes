// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: the value of an assignment to a bool is the converted value, 0 or 1
#include <cerastes.h>

int flag(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? 2 : 0))
{
  bool b;
  int t = (b = a) ? 1 : 0;
  return t + (b ? 1 : 0);
}
