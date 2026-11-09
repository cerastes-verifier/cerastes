// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (retires reject_owns_block_load.c's shape, owns(*pp) && owns_block(*pp)): an ensures site owns_block(*pp) with no requires partner GIVES the caller the block the callee allocated into *pp (the callee holds the whole block at its return, no amount: 0052 item 2); the caller frees *pp; the pointer cell *pp is the partnered owns(*pp), not "beside" the block site (different keys)
// Pre-registered from record 0054 item 2 ("given"): verifies in both modes (S39 (a), tests/viper-smoke/memory/s39_a_given_block.vpr)
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp) && **pp == 5)
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 5)
{
  int *p = NULL;
  make(&p);
  int v = *p;
  free(p);
  return v;
}
