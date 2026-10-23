// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 2: a void call may be the left operand of a comma, and its precondition is checked
#include <cerastes.h>

void touch(int v)
    CERASTES_REQUIRES(v >= 0)
{
  CERASTES_CHECK(v >= 0);
  (void)v;
}

int after(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return (touch(x), x + 1);
}
