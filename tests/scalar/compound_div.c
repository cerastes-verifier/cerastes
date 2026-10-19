// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 7 and 0012 item 7: /= and %= are divisions with their checks
#include <cerastes.h>

int shrink(int a, int b)
    CERASTES_REQUIRES(b > 0)
    CERASTES_ENSURES(r, r == (a / b) % b)
{
  a /= b;
  a %= b;
  return a;
}
