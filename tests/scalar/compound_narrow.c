// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 items 5 and 6: c += 1 on signed char computes in int and converts back (checked)
// The ensures reads the parameter's entry value (0006 item 5, 0012 item 2).
#include <cerastes.h>

signed char bump(signed char c)
    CERASTES_REQUIRES(c >= -100 && c <= 100)
    CERASTES_ENSURES(r, r == c + 1)
{
  c += 1;
  return c;
}
