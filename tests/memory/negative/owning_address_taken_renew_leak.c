// expect: error leak @ L27
// mode: c23, c++20
// twin: tests/memory/owning_address_taken_renew.c
// Pins: 0054 item 6: the caller keeps the block renew returned (it may be non-null) and never frees it: v's exit row
// Pre-registered from record 0054 item 6 (S39 f1y): leak at the return statement
#include <cerastes.h>
#include <stdlib.h>

void renew(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  free(*pp);
  *pp = (int *)malloc(sizeof(int));
  if (*pp != NULL) {
    **pp = 1;
  }
}

int use_renew(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = NULL;
  renew(&v);
  renew(&v);

  return 0;  // <- here
}
