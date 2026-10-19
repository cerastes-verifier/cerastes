// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3: signed subtraction
// a - b with both non-negative cannot leave int.
#include <cerastes.h>

int sub(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r <= a)
{
  return a - b;
}
