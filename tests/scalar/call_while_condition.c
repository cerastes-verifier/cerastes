// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (from reject_call_loop_condition.c): a call in a while condition runs at every evaluation, the last one included; its precondition holds at each
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int count(int n)
    CERASTES_REQUIRES(0 <= n && n < 2147483647)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (next(i) <= n) {
    i = i + 1;
  }
  return i;
}
