// expect: error division-by-zero @ L21
// mode: c23, c++20
// twin: tests/scalar/never_returns_call.c
// Pins: 0021 soundness pin: nr(k) + 100 / zero(y): C may evaluate the division before the call that never returns, so the division by zero is an error
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int zero(int y)
    CERASTES_ENSURES(r, r == 0)
{
  return y - y;
}

int divide(int k, int y)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, false)
{
  return never_returns(k) + 100 / zero(y);
}
