// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 3: static_assert is checked by EDG and not encoded
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  return x + 1;
}
