// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 item 4: g reaches f only through dead code, and Viper's call graph counts that edge, so f and g are one component; the call g(n - 1) in f decreases
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false);

int f(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + g(n - 1);
}

int g(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (0) {
    return f(n);
  }
  return never_returns(0);
}
