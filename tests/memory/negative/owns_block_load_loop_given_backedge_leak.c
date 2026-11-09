// expect: error leak @ L30
// expect: error invalid-free @ L31
// mode: c23, c++20
// twin: tests/memory/owns_block_load_loop_given.c
// Pins: 0054 item 2 ("The back-edge leak trigger counts calls whose callee gives a site through a load") and audit round 2 item 2 (probe l6): an iteration ends owning the block make gave and the loop does not carry it (and the next make overwrites it): leak at the back edge; the free after the loop then finds no block it holds
// Pre-registered from the audit probe l6_loop_given_backedge_leak as observed: leak at the loop body's closing brace and, as a consequence, invalid-free at the free after the loop
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp != NULL) {
    **pp = 1;
  }
}

void f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10)
{
  int *p = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    make(&p);
    i += 1;
  }  // <- here
  free(p);  // <- here
}
