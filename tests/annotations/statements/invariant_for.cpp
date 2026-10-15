// expect: golden
// expect: native-ok
// Pins: invariant on a for statement; the counter is declared BEFORE the loop
// (decision 0006 item 2: a for-init variable is not in scope in the attribute).
#include <cerastes.h>

int count(int n) CERASTES_REQUIRES(n >= 0)
{
  int i;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == i) for (i = 0; i < n; ++i) {
    c += 1;
  }
  return c;
}
