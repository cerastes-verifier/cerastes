// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 items 2, 8: malloc's cell holds no value (its view is ref_to_uninit), so a read before the first write is `uninitialized`
// Pre-registered from record 0047 item 8 (malloc: the flags are unknown): uninitialized at the read; free needs no value, so nothing else; the ensures holds of any value, since returning the unwritten value would otherwise add a postcondition error (re-registered after the first run)
#include <cerastes.h>
#include <stdlib.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;  // <- here
  free(p);
  return v;
}
