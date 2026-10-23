// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/do_incr_condition.c
// Pins: 0020 item 3: the body runs n + 1 times
#include <cerastes.h>

int count_do(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827881)
    CERASTES_ENSURES(r, r == 3 * n + 2)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(i <= n && total == 3 * i)
  CERASTES_DECREASES(n - i)
  do {
    total += 3;
  } while (i++ < n);
  return total;
}
