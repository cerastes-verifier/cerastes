// expect: error uninitialized @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_malloc.c
// Pins: 0064 item 15 (malloc: the flags unknown, 0047 item 8): an element read before any write is uninitialized, at the read; free needs no value; the ensures holds of any value (the precedent negative/heap_read_before_write.c)
// Pre-registered from record 0064 item 15 and the precedent heap_read_before_write.c (uninitialized at the read), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int read_first(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  int v = p->a[1];  // <- here
  free(p);
  return v;
}
