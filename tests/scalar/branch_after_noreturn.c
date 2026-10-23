// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 soundness pin: a branch with a call (a > 0 && 100 / g(a) > 0) is placed before never_returns(k), because C may evaluate it first; the guard keeps its division safe
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int g(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

int test(int k, int a)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, false)
{
  return never_returns(k) + (a > 0 && 100 / g(a) > 0);
}
