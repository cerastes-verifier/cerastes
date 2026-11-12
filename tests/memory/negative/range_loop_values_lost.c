// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/range_loop_values.c
// Pins: 0060 item 11: after a loop that writes every element of an owned range the old value is gone (no values-kept conjunct for a written field): the check of p[0] == 7 fails at the check, one error
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

int zero_all(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0] == 7)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    p[i] = 0;
    i++;
  }
  CERASTES_CHECK(p[0] == 7);
  return 0;
}
