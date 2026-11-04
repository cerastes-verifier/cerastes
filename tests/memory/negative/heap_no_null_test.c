// expect: error null-dereference @ L13
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 2: malloc's contract is `r == NULL || owns_block(r)`, so the result may be null and `*p` without a test applies * to null
// Pre-registered from record 0047 item 2 (the view's `r == NULL ||`): null-dereference at the write, nothing else (the null-dereference row precedes the permission row and the path continues with p != null)
#include <cerastes.h>
#include <stdlib.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  *p = 7;  // <- here
  free(p);
  return 7;
}
