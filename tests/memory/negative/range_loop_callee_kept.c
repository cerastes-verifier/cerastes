// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/range_loop_callee.c
// Pins: 0060 item 11 (S44 (g) keep; ruling in report 16): keep(&p[i]) never gives the element back: the first marked back-edge row fails, permission at the closing brace of the loop body, where 0038 item 4 and 0059 item 8 put the back-edge rows (as e1a's array_loop_callee_keep)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

void give(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q));

void fill5(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_DECREASES(n - i) while (i < n) {
    give(&p[i]);
    i++;
  }
}
