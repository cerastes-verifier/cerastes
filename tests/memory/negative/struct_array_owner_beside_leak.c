// expect: error leak @ L26
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_beside.c
// Pins: 0064 item 15 and 0055 item 4: the owned block of the member is never freed (free(b) alone, with the array member beside it): leak at the closing brace (the precedent negative/owning_field_alloc_free_with_block.c)
// Pre-registered from record 0064 item 15 and the precedent owning_field_alloc_free_with_block.c (leak at the closing brace), before any run: unmeasured
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct B {
  int a[4];
  int *p CERASTES_OWNING;
};

int leaky(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct B *b = (struct B *)calloc(1, sizeof(struct B));
  if (b == NULL) {
    return -1;
  }
  b->a[1] = 3;
  b->p = (int *)malloc(sizeof(int));
  free(b);
  return 0;
}  // <- here
