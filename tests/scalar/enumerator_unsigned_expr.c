// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 2: an enumerator initializer is checked as written, here an unsigned subtraction that fits
#include <cerastes.h>

enum {
  A = (5u - 1u) > 3u
};

int a_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return A;
}
