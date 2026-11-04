// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 items 4, 6: free consumed the cell, so a later access `*p` is `permission` (0028 item 6: any access after free)
// Pre-registered from record 0047 item 6 (after the call the block is consumed; any access is `permission`): permission at the read
#include <cerastes.h>
#include <stdlib.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  free(p);
  int v = *p;  // <- here
  return v;
}
