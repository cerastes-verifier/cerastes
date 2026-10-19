// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5: an initializer converting to a signed type is checked
#include <cerastes.h>

short narrow_init(int x)
    CERASTES_REQUIRES(x >= -32768 && x <= 32767)
{
  short s = x;
  return s;
}
