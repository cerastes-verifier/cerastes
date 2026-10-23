// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021 (from reject_do_call_condition.c): a call in a do condition runs after each body, the last one included; its precondition holds at each
#include <cerastes.h>

int step(int x)
    CERASTES_REQUIRES(x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int test(int n)
    CERASTES_REQUIRES(n <= 100)
    CERASTES_ENSURES(r, r == (n > 1 ? n - 1 : 1))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && (i == 0 || i + 2 <= n))
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
  } while (step(i) < n);
  return i;
}
