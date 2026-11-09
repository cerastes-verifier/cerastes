// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 6 (the callee's ensures site on *pp under a guard) and audit round 2 (Parts C and D), probe c4: maybe() consumes the block when c == 0 (the guard says nothing then, so v dangles and held_v is false) and returns it when c != 0; the caller frees it on that path only
// Pre-registered from record 0054 item 6 (after a call held_v := held_v && perm(token) > none, so a consumed block leaves no owner): verifies in both modes (audit probe c4; exit 3 / uninitialized when written; the record does not say more, see the report)
#include <cerastes.h>
#include <stdlib.h>

void maybe(int **pp, int c)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns_block(*pp)))
    CERASTES_ENSURES(cerastes_owns(*pp) && (c == 0 || (*pp == NULL || cerastes_owns_block(*pp))))
{
  if (c == 0) {
    free(*pp);
  }
}

int use(int c)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v != NULL) {
    *v = 0;
  }
  maybe(&v, c);
  if (c != 0) {
    free(v);
  }
  return 0;
}
