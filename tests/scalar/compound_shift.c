// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 items 6 and 8: c <<= 1 on signed char shifts in int and converts back (checked)
#include <cerastes.h>

signed char double_c(signed char c)
    CERASTES_REQUIRES(c >= 0 && c <= 63)
    CERASTES_ENSURES(r, r == c + c)
{
  c <<= 1;
  return c;
}
