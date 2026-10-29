// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 7: a break out of a loop body that declares a cell (the scope exit at the break; replaces reject_loop_cell's cousin reject_return_in_loop_with_cell's shape for break)
#include <cerastes.h>

int stop_at(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r <= n && r <= 5)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && i <= 5)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int x = i;
    int *p = &x;
    if (*p == 5) {
      break;
    }
    i += 1;
  }
  return i;
}
