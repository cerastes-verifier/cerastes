// expect: error termination @ L19
// mode: c23, c++20
// twin: tests/scalar/recursive_after_noreturn.c
// Pins: 0021 soundness pin: nr(0) + h(n): C may call h(n) first, which never terminates; Silicon alone never checks the call, because the path ends at never_returns
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int h(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, false)
{
  if (n == 0) {
    return never_returns(0);
  }
  return never_returns(0) + h(n);
}
