// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 6b (the former pin reject_owns_block_guard_heap_ensures.c): the result is a block exactly where *q > 0, read in the post-state with *q == old(*q); the caller frees it when it holds it
// Pre-registered from record 0052 item 6b: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int *make(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == cerastes_old(*q) && (*q > 0 ? cerastes_owns_block(r) : r == NULL))
{
  if (*q > 0) {
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      abort();
    }
    *p = *q;
    return p;
  }
  return NULL;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 0)
{
  int c = 3;
  int *p = make(&c);
  if (p != NULL) {
    free(p);
  }
  return 0;
}
