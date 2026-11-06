// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 6b (amends 0047 item 4; the former pin reject_owns_block_guard_heap_requires.c): a block site whose guard reads the heap in a requires: the callee owns p's block exactly where *q > 0 and frees it there; the caller passes a block with c == 1 and NULL with d == 0
// Pre-registered from record 0052 item 6b: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

void maybe_free(int *q, int *p)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 0 ? cerastes_owns_block(p) : true))
    CERASTES_ENSURES(cerastes_owns(*q) && *q == cerastes_old(*q))
{
  if (*q > 0) {
    free(p);
  }
}

int use_guard(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  int c = 1;
  maybe_free(&c, p);
  int d = 0;
  maybe_free(&d, NULL);
  return 0;
}
