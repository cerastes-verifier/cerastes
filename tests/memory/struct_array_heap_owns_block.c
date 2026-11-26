// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (heap storage; the pin struct_array_reject_heap.c's f_d03 and f_d04 are s_free and s_touch): owns_block of a structure with an array member in a constructor's ensures (framing an element read), in a destructor's requires (alone: the block is taken) and in a borrower's requires and ensures; the caller writes an element and reads the borrower's write
// Pre-registered from record 0064 item 15 (Part D) and the precedent heap_owns_block_value.c, before any run: verifies (unmeasured: an element read framed by owns_block has no precedent)
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
  free(p);
}

void s_touch(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p) && p->n == 3)
{
  p->n = 3;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S *p = s_new();
  CERASTES_CHECK(p->a[1] == 0);
  p->a[2] = 5;
  CERASTES_CHECK(p->a[2] == 5);
  s_touch(p);
  CERASTES_CHECK(p->n == 3);
  s_free(p);
  return 0;
}
