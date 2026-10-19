// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 6: a call asserts the callee's requires, at the call
#include <cerastes.h>

int positive(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int use(int y)
    CERASTES_REQUIRES(y > 5)
{
  return positive(y);
}
