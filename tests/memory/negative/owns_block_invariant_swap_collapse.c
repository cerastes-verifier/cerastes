// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_swap.c
// Pins: 0054 item 4 (S39 c1y): the body makes both pointers name one block: the pair row p != null && q != null ==> block_of(p) != block_of(q) fails at the back edge (permission, never Silicon's own invariant check: the held rows are checks, not exhales)
// Pre-registered from record 0054 item 4 ("Pairs"): permission at the loop body's closing brace (the back-edge position of the existing pins)
#include <cerastes.h>
#include <stdlib.h>

int swapper(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns_block(p)) && (q == NULL || cerastes_owns_block(q)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    i += 1;
  }  // <- here
  free(p);
  free(q);
  return 0;
}
