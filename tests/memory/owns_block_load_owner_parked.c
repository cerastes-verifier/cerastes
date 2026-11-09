// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("The leak exemption", 0052 item 3 "Owners") and audit round 2 item 2 (probe o1): an owning local that is parked in *pp before the return is a returned owner: the ensures site on *pp hands its block to the caller, so the end of the function owes nothing
// Pre-registered from the audit probe o1_owner_parked (verifies today): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v != NULL) {
    *v = 1;
  }
  *pp = v;
}
