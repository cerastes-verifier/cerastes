// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 12: a reference to a local const variable is that local
#include <cerastes.h>

int scaled(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 10)
    CERASTES_ENSURES(r, r <= 1000 && r == x * 100)
{
  const int k = 100;
  return x * k;
}
