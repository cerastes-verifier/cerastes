// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: x++ as an argument passes the old value and stores the new one before the call
#include <cerastes.h>

int id(int v)
    CERASTES_REQUIRES(v >= 0)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int take(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * x + 1)
{
  int a = id(x++);
  return a + x;
}
