// expect: error leak @ L31
// mode: c23, c++20
// twin: tests/memory/block_guard_heap_ensures.c
// Pins: 0052 item 6b: the caller knows the guard holds (c == 3 is unchanged) and does not free the block it received: leak at its closing brace
// Pre-registered from record 0052 item 6b: leak at the closing brace
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
  (void)p;
  return 0;
}  // <- here
