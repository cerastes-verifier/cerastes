// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 and audit round 2 item 2 (probe p5): a block site on a formal (p) and one through a load (*pp), the second tied to its entry block: a bodiless contract the caller satisfies with two distinct blocks
// Pre-registered from the audit probe p5_two_sites_formal_and_load (failed with a false alarm when written; the il-analyst fixes it): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void f(int *p, int **pp)
    CERASTES_REQUIRES(cerastes_owns_block(p) && cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns_block(p) && cerastes_owns(*pp) && cerastes_owns_block(*pp) && *pp == cerastes_old(*pp));

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return 1;
  }
  *a = 1;
  *b = 2;
  f(a, &b);
  free(a);
  free(b);
  return 0;
}
