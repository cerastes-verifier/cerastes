// expect: error permission @ L30
// mode: c23, c++20
// twin: tests/memory/owns_block_load_loop_borrow.c
// Pins: 0054 item 2 ("Loops") and audit round 2 item 2 (probe l5d): the loop frees the borrowed block on one iteration: an iteration ends without the permission to **pp that it began with
// Pre-registered from the audit probe l5d_free_in_loop as observed: permission at the loop body's closing brace
#include <cerastes.h>
#include <stdlib.h>

void touch(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && *pp == cerastes_old(*pp))
{
  **pp = 2;
}

void f(int **pp, int n)
    CERASTES_REQUIRES(0 <= n && n <= 10 && cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int *keep = *pp;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *pp == keep)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    touch(pp);
    if (i == 1) {
      free(*pp);
    }
    i += 1;
  }  // <- here
  free(keep);
}
