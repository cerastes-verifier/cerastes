// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// Pins: 0058 item 3 (audit S6): the canonical form that satisfies both strict options, CERASTES_OWNING CERASTES_REF_TO_UNINIT int *v = malloc(n): --strict-init asks the receiver of malloc's result to be marked ref_to_uninit, --strict-owner asks it to be an owner; the two allowance sets are independent
// Pre-registered from record 0058 item 3 (audit round 1, S6): verifies in both modes under --strict-init --strict-owner (build/m4.5d2b2d/emul3/s6b.c verifies today)
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
  return r;
}
