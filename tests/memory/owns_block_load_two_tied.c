// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("Bodiless contracts": "each pair of ensures block sites, one of which loads, gets 0045 item 5's distinctness assert") and audit round 2 item 2 (probe p10): two block sites through two loads, both tied to their entry blocks (*pp == old(*pp), *qq == old(*qq)): their requires sites are distinct blocks, so the pair need not be re-proved distinct as new blocks
// Pre-registered from the audit probe p10_two_loads_tied (FAILED with a false alarm when written: "may be the same block"; the il-analyst fixes it): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void both(int **pp, int **qq)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns_block(*pp) && cerastes_owns_block(*qq))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(*qq) && cerastes_owns_block(*pp) && cerastes_owns_block(*qq) && *pp == cerastes_old(*pp) && *qq == cerastes_old(*qq));

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
  both(&a, &b);
  free(a);
  free(b);
  return 0;
}
