// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 9: each return is checked against the ensures
#include <cerastes.h>

int clamp(int x, int lo, int hi)
    CERASTES_REQUIRES(lo <= hi)
    CERASTES_ENSURES(r, r >= lo && r <= hi)
{
  if (x < lo) {
    return lo;
  } else if (x > hi) {
    return hi;
  }
  return x;
}
