// expect: error invalid-free @ L17
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0059 item 10 (0047 item 6): free(&p[1]) is not the start of the allocation (offset 1 of a one-object block): invalid-free at the call
// Pre-registered from record 0059 item 10 (Part D), before the code
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
  free(&p[1]);
  return 7;
}
