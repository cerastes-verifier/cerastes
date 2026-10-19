// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0011 item 5 and 0012 item 6: a call is verified against the callee's contract, not its body
// twice has body x * 2 but promises only r >= x; the caller may use no more.
#include <cerastes.h>

int twice(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 1000)
    CERASTES_ENSURES(r, r >= x)
{
  return x * 2;
}

int use(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 1000)
    CERASTES_ENSURES(r, r >= x)
{
  return twice(x);
}
