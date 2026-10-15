// expect: golden
// expect: native-ok
// Pins: invariant on a while statement (prefix); the attribute is on the
// stmk_while node and its condition sees the locals declared before the loop.
#include <cerastes.h>

int sum_to(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s >= 0) while (i < n) {
    s += i;
    i += 1;
  }
  return s;
}
