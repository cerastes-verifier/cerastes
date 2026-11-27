// expect: error leak @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_member.c
// Pins: 0064 item 15 and 0055 item 4 (free of a structure while its owning member holds a block): the destructor frees the structure and not the structure its owning member points to: leak at the closing brace of r_destroy (the precedent negative/owning_field_consume_free_parent.c)
// Pre-registered from record 0064 item 15 and the precedent owning_field_consume_free_parent.c (leak at the closing brace), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct R {
  CERASTES_OWNING struct S *q;
  int z;
};

void r_destroy(struct R *x)
    CERASTES_REQUIRES(cerastes_owns_block(x))
{
  free(x);
}  // <- here

int build(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct R *b = (struct R *)calloc(1, sizeof(struct R));
  if (b == NULL) {
    return -1;
  }
  b->q = (struct S *)calloc(1, sizeof(struct S));
  if (b->q != NULL) {
    b->q->a[1] = 5;
  }
  r_destroy(b);
  return 0;
}
