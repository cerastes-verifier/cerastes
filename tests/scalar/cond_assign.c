// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 3: the arms of ?: may assign; only the chosen arm runs
#include <cerastes.h>

int pick(int c, int a, int b)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823 && -1073741824 <= b && b <= 1073741823)
    CERASTES_ENSURES(r, r == (c != 0 ? 2 * a : 2 * b))
{
  int x;
  int y = c != 0 ? (x = a) : (x = b);
  return y + x;
}
