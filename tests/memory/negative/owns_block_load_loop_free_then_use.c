// expect: error permission @ L23
// expect: error leak @ L24
// mode: c23, c++20
// twin: tests/memory/owns_block_load_loop.c
// Pins: 0054 item 2 ("Loops") and audit round 2 item 2 (probe l1): the loop frees the block on its first iteration and writes **pp on the next: an iteration ends without the permission it began with
// Pre-registered from the audit probe l1_loop_free_then_use as observed: permission at the loop body's closing brace (the back-edge row) and, as a consequence of the same fault, a leak at the function's closing brace
#include <cerastes.h>
#include <stdlib.h>

void f(int **pp, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10 && cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i;
    if (i == 0) {
      free(*pp);
    }
    i += 1;
  }  // <- here
}  // <- here
