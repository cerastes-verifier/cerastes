// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5: a return value converting to a signed type is checked
#include <cerastes.h>

signed char fit(int x)
    CERASTES_REQUIRES(x >= -128 && x <= 127)
{
  return x;
}
