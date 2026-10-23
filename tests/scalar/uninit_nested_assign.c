// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 13: an assignment used as a value gives the variable a value for what follows it
#include <cerastes.h>

int nested(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  int x;
  int y = (x = a) + 1;
  return y + x;
}
