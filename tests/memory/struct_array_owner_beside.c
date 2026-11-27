// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (293; the pin struct_array_reject_members.c L21's structure is B): an owning member beside an array member of scalars: allocate, use both, destroy; and a function that frees the owned block, clears the member and then reads an element (an array leaf is never an owner, so it stays held)
// Pre-registered from record 0064 item 15 and the precedents owning_field_alloc.c and the clear_inner shape of strict_owner_p6.c, before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct B {
  int a[4];
  int *p CERASTES_OWNING;
};

void b_destroy(struct B *b)
    CERASTES_REQUIRES(b == NULL || cerastes_owns_block(b))
{
  if (b != NULL) {
    free(b->p);
    free(b);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == 7 || r == -1)
{
  struct B *b = (struct B *)calloc(1, sizeof(struct B));
  if (b == NULL) {
    return -1;
  }
  b->a[1] = 3;
  b->p = (int *)malloc(sizeof(int));
  int v = 0;
  if (b->p != NULL) {
    *b->p = 4;
    v = *b->p + b->a[1];
  }
  b_destroy(b);
  return v;
}

int array_after_owned_freed(void)
    CERASTES_ENSURES(r, r == 6 || r == -1)
{
  struct B *b = (struct B *)calloc(1, sizeof(struct B));
  if (b == NULL) {
    return -1;
  }
  b->a[2] = 6;
  b->p = (int *)malloc(sizeof(int));
  free(b->p);
  b->p = NULL;
  int v = b->a[2];
  free(b);
  return v;
}
