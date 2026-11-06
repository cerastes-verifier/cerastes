// expect: error leak @ L33
// mode: c23, c++20
// twin: tests/memory/block_guard_old.c
// Pins: 0052 item 6b: the caller does not free the block that the old-value guard hands it: leak at its closing brace
// Pre-registered from record 0052 item 6b: leak at the closing brace
#include <cerastes.h>
#include <stdlib.h>

int *make(int *q)
    CERASTES_REQUIRES(cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_owns(*q) && *q == 0 && (cerastes_old(*q) > 0 ? cerastes_owns_block(r) : r == NULL))
{
  int v = *q;
  *q = 0;
  if (v > 0) {
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      abort();
    }
    *p = v;
    return p;
  }
  return NULL;
}

int use_make(void)
    CERASTES_ENSURES(r, r == 0)
{
  int c = 3;
  int *p = make(&c);
  (void)p;
  return c;
}  // <- here
