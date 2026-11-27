// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (293; 0055: the closure of an atom over an owning field; the pin struct_array_reject_marked_members.c's R_d12 is struct R): an owning member that points to a structure with an array member: the destructor frees the member then the structure, the builder fills an element and the other member
// Pre-registered from record 0064 item 15 and the precedent owning_field_alloc.c, before any run: verifies (unmeasured: a closure over a block whose structure has an array leaf)
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
    CERASTES_REQUIRES(x == NULL || cerastes_owns_block(x))
{
  if (x != NULL) {
    free(x->q);
    free(x);
  }
}

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
    b->q->n = 1;
  }
  r_destroy(b);
  return 0;
}
