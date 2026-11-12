// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/range_loop_guarded.c
// Pins: 0060 item 11: a second ensures claims p[0] == 0 under the same guard: the loop wrote p[i] = i and keeps no value, so the postcondition fails at the closing brace, one error
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

void fill_g(int *p, int n)
    CERASTES_REQUIRES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(n == 0 || cerastes_owns_range(p, n))
    CERASTES_ENSURES(n == 0 || p[0] == 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    p[i] = i;
    i++;
  }
}
