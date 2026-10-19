// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5 and 0012 item 6: a call result converts to the return type, checked from the callee's ensures
#include <cerastes.h>

int small(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= 0 && r <= 100)
{
  return x > 100 ? 100 : x;
}

signed char narrow_result(int x)
    CERASTES_REQUIRES(x >= 0)
{
  return small(x);
}
