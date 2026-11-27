// expect: error invalid-free @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_malloc.c
// Pins: 0064 item 15 (free's rows are per leaf, `invalid-free`): the second free of a structure with an array member finds no token: invalid-free at the second call, once (the precedent negative/heap_double_free.c; the per-leaf rows of one call print once)
// Pre-registered from record 0064 item 15 and the precedent heap_double_free.c (invalid-free at the second free only), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int twice(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 7;
  free(p);
  free(p);  // <- here
  return 7;
}
