// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 5: a comma under a (void) cast may end in an operand without effect
#include <cerastes.h>

int bump(int x, int y)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  (void)(x++, y);
  return x;
}
