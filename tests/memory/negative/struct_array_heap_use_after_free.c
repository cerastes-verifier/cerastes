// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_malloc.c
// Pins: 0064 item 15 (free's rows per leaf; 0047 item 6): an element read after free is permission, at the read (the precedent negative/heap_use_after_free.c)
// Pre-registered from record 0064 item 15 and the precedent heap_use_after_free.c (permission at the read), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int after_free(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[1] = 7;
  free(p);
  int v = p->a[1];  // <- here
  return v;
}
