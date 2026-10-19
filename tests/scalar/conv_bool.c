// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5: a conversion to bool is v != 0
#include <cerastes.h>

int truth(int x)
    CERASTES_ENSURES(r, r == (x != 0))
{
  bool b = x;
  return b;
}
