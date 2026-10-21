// expect: error postcondition @ L41
// mode: c23, c++20
// twin: tests/scalar/loop_in_switch.c
// Pins: 0014 item 6: the code after the loop inside the case runs after the loop's break
// For n > 5 the loop breaks with total == 10 and total += 1 still runs.
#include <cerastes.h>

int run(int mode, int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (mode == 1 && r == n) || (mode == 2 && ((n <= 5 && r == 2 * n + 1) || (n > 5 && r == 10))) || (mode != 1 && mode != 2 && r == 9))
{
  int total = 0;
  int i = 0;
  switch (mode) {
  case 1: {
    CERASTES_INVARIANT(i <= n && total == i)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      total += 1;
      i += 1;
    }
    break;
  }
  case 2: {
    CERASTES_INVARIANT(i <= n && i <= 5 && total == 2 * i)
    CERASTES_DECREASES(n - i)
    while (i < n) {
      if (i == 5) {
        break;
      }
      total += 2;
      i += 1;
    }
    total += 1;
    break;
  }
  default:
    total = 9;
    break;
  }
  return total;
}
