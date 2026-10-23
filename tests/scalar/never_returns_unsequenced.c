// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 7 and 0012 item 6: a callee that never returns does not hide a check that C may evaluate first; with the divisor known non-zero the checks hold
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int divide(int k, int n, int y)
    CERASTES_REQUIRES(0 <= k && y != 0 && y != -1 && y < 2147483647)
    CERASTES_ENSURES(r, false)
{
  return never_returns(k) + n / y++;
}

int comma_divide(int k, int n, int d)
    CERASTES_REQUIRES(0 <= k && d != -1 && d != -2 && d < 2147483647)
    CERASTES_ENSURES(r, false)
{
  return never_returns(k) + (d++, n / d);
}
