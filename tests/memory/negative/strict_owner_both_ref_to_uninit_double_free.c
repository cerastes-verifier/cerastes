// expect: error invalid-free @ L20
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// twin: tests/memory/strict_owner_both_ref_to_uninit.c
// Pins: 0058 item 3 (audit S6): the default mode's checks stay under both options: the owner is released twice
// Pre-registered from record 0058 item 3 (audit round 1, S6): one invalid-free at the second free
#include <cerastes.h>
#include <stdlib.h>

int use(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  CERASTES_OWNING CERASTES_REF_TO_UNINIT int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 3;
  int r = *v;
  free(v);
  free(v);  // <- here
  return r;
}
