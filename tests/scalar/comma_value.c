// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: the value of a comma operator is its right operand, after the effects of its left operand
#include <cerastes.h>

int seq(int a, int b)
    CERASTES_REQUIRES(0 <= b && a <= 2147483647 - b)
    CERASTES_ENSURES(r, r == a + b)
{
  int x;
  int y = (x = a, x + b);
  return y;
}
