// expect: golden
// expect: native-ok
// Pins: invariant on a do-while statement (stmk_end_test_while).
#include <cerastes.h>

int at_least_one(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n + 1) do {
    i += 1;
  } while (i < n);
  return i;
}
