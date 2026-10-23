// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: an effect in the right operand of && runs only when the left operand is true
#include <cerastes.h>

int small(int a)
    CERASTES_ENSURES(r, r == ((0 < a && a < 5) ? a + 1 : 0))
{
  int x;
  if (a > 0 && (x = a) < 5) {
    return x + 1;
  }
  return 0;
}
