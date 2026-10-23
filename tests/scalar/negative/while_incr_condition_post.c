// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/while_incr_condition.c
// Pins: 0020 item 3: the loop ends with i == n + 1 and total == 2 * n
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n <= 715827882)
    CERASTES_ENSURES(r, r == 3 * n)
{
  int i = 0;
  int total = 0;
  CERASTES_INVARIANT(i <= n && total == 2 * i)
  CERASTES_DECREASES(n - i)
  while (i++ < n) {
    total += 2;
  }
  return total + i;
}
