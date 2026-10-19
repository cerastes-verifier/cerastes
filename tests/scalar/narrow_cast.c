// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5: an explicit cast to a signed type is checked, not modular
#include <cerastes.h>

signed char narrow(int x)
    CERASTES_REQUIRES(x >= -128 && x <= 127)
    CERASTES_ENSURES(r, r == x)
{
  return (signed char)x;
}
