// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/heap_calloc_int.c
// Pins: 0047 item 2: the view's value clause is 0, not anything else
// Pre-registered from record 0047 item 2 (calloc's zeroed clause): check at the CERASTES_CHECK
#include <cerastes.h>
#include <stdlib.h>

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;
  CERASTES_CHECK(v == 1);  // <- here
  free(p);
  return v;
}
