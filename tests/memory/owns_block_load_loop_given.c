// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("Loops": "calls whose callee gives a site through a load") and audit round 2 item 2 (probe l6): each iteration receives a block through make(&p) and frees it before the back edge; p is reset to NULL, so the block after the loop is none
// Pre-registered from the audit probe l6 with the free and reset added and p == NULL in the invariant (derived from the rules, not observed): verifies in both modes; if it does not, ask before changing it
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
  CERASTES_INVARIANT(0 <= i && i <= n && p == NULL)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    make(&p);
    free(p);
    p = NULL;
    i += 1;
  }
  free(p);
}
