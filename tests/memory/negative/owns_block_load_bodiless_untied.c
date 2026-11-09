// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/owns_block_load_bodiless_tied.c
// Pins: 0054 item 2 (audit round 1 B1): a bodiless borrowed site through a load that does not say it names its partner's entry block could return a block the caller already holds a second token for: f__wf's tie fails, "the contract of rekey returns the block of *pp without saying it is the one its requires took (write *pp == old(*pp))"
// Pre-registered from record 0054 item 2 ("The tie"): permission at the ensures clause with the site (LINE is a best reading: the clause, as 0045 item 4's f__wf errors)
#include <cerastes.h>
#include <stdlib.h>

void rekey(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp));  // <- here

int use_rekey(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  rekey(&p);
  free(p);
  return 0;
}
