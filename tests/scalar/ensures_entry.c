// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0006 item 5 and 0012 item 2: ensures reads the entry value of a reassigned parameter
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  x = x + 1;
  return x;
}
