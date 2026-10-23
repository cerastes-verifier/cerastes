// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: a comma operator whose value is discarded is the statements of its operands, in order
#include <cerastes.h>

int pair(int a, int b)
    CERASTES_REQUIRES(0 <= b && a <= 2147483647 - b)
    CERASTES_ENSURES(r, r == a + b)
{
  int x;
  int y;
  x = a, y = b;
  return x + y;
}
