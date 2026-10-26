// expect: error division-by-zero @ L19
// mode: c23, c++20
// twin: tests/scalar/call_two_writes.c
// Pins: 0025 item 2 soundness pin (the analogue of 0021 item 11 for a store): the store of a result that never returns leaves the chain, but C may run zero(y) first and divide by zero before the call that never returns; the division's check must not be placed after the store
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int zero(int y)
    CERASTES_ENSURES(r, r == 0);

int test(int k, int y)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, false)
{
  int x;
  int s = (x = never_returns(k)) + 100 / zero(y);
  return s + x;
}
