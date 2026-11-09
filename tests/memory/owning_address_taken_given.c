// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 (&v passed to a callee that only gives a block, an ensures site on *pp alone): after the call v owns the block (held_v := v != null && offset_of(v) == 0 && the token at write), and the owner frees it
// Pre-registered from record 0054 item 6 (audit item 11): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void make(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 5;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = NULL;
  make(&v);
  free(v);
  return 0;
}
