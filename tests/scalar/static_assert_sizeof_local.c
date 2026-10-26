// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 2: the local-name rejections of the case-value path apply to names the condition evaluates, not to the operand of sizeof
#include <cerastes.h>

int widths(int n)
    CERASTES_REQUIRES(n <= 2147483643)
    CERASTES_ENSURES(r, r == n + 4)
{
  int x = n;
  static_assert(sizeof(x) == sizeof(int), "sizeof does not read x");
  return x + (int)sizeof(x);
}
