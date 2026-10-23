// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (from reject_effect_call_loop_condition.c): the assignment of a call's result in a loop condition runs at every evaluation; the last value of c is read after the loop
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int f(int n)
    CERASTES_REQUIRES(1 <= n && n <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * n)
{
  int i = 1;
  int c;
  CERASTES_INVARIANT(1 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while ((c = pos(i)) < n) {
    i++;
  }
  return i + c;
}
