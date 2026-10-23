// expect: error division-by-zero @ L21
// mode: c23, c++20
// twin: tests/scalar/branch_after_noreturn.c
// Pins: 0021 soundness pin: the guard a >= 0 lets a == 0 divide by zero in the branch, which C may run before the call that never returns
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
  return never_returns(k) + (a >= 0 && 100 / g(a) > 0);
}
