// expect: error invalid-free @ L29
// mode: c23, c++20
// twin: tests/memory/owns_block_load_loop_borrow.c
// Pins: 0054 item 2 ("Loops") and audit round 2 item 2 (probe l5c): keep names the block *pp holds, which the first free already ended
// Pre-registered from the audit probe l5c_double_free as observed: invalid-free at the second free
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
    i += 1;
  }
  free(keep);
  free(*pp);  // <- here
}
