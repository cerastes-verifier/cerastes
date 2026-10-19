// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3: signed multiplication of two variables (non-linear for Z3)
// 40000 * 40000 = 1.6e9 fits int.
#include <cerastes.h>

int area(int w, int h)
    CERASTES_REQUIRES(w >= 0 && w <= 40000 && h >= 0 && h <= 40000)
    CERASTES_ENSURES(r, r >= 0)
{
  return w * h;
}
