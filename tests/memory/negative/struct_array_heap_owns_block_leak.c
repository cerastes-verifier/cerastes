// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/struct_array_heap_owns_block.c
// Pins: 0064 item 15: a destructor that takes the block and neither frees it nor hands it back leaks it: leak at the closing brace (the precedent negative/heap_consume_leak.c)
// Pre-registered from record 0064 item 15 and the precedent heap_consume_leak.c (leak at the closing brace), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct S *s_new(void)
    CERASTES_ENSURES(r, cerastes_owns_block(r) && r->a[1] == 0 && r->n == 0)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    abort();
  }
  return p;
}

void s_free(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
}  // <- here
