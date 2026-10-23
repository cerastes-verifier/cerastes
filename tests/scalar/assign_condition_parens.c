// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 4: an assignment used as a truth value is written with its own parentheses and a comparison
#include <cerastes.h>

int clamp(int a)
    CERASTES_REQUIRES(-2147483647 <= a)
    CERASTES_ENSURES(r, r == (a - 1 > 0 ? a - 1 : -1))
{
  int x;
  if ((x = a - 1) > 0) {
    return x;
  }
  return -1;
}

int nonzero(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? a : -1))
{
  int x;
  if ((x = a) != 0) {
    return x;
  }
  return -1;
}
