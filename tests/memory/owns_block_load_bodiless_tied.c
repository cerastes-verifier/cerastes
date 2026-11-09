// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("Bodiless contracts"): a bodiless borrowed site through a load is accepted when its ensures says it names the entry block (*pp == old(*pp)): nothing proves a bodiless ensures, so the tie keeps a callee from returning another block the caller already holds
// Pre-registered from record 0054 item 2: verifies (the tie holds in f__wf)
#include <cerastes.h>
#include <stdlib.h>

void rekey(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && *pp == cerastes_old(*pp));

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
