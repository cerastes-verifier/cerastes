// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_malloc.c
// Pins: 0064 item 15 (the leak check reads the block's token as before, 0047 item 7): one path returns with the block held: leak at the function's closing brace (the precedent negative/heap_leak_no_free.c)
// Pre-registered from record 0064 item 15 and the precedent heap_leak_no_free.c (leak at the closing brace), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int leaky(int c)
    CERASTES_ENSURES(r, r == 0 || r == 1 || r == -1)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  if (c != 0) {
    p->a[0] = 1;
    return 1;
  }
  free(p);
  return 0;
}  // <- here
