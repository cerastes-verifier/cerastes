// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 (&v passed to a callee that borrows the block): owns(*pp) && (*pp == NULL || owns_block(*pp)) in both lists lends v's block to renew, which replaces it; the owner frees what it holds afterwards
// Pre-registered from record 0054 item 6 (S39 (f), s39_f_address_taken_owner.vpr): verifies in both modes
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
  free(v);
  return 0;
}
