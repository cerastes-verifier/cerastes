// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-owner
// Pins: 0064 item 15 (293) under --strict-owner (0058 item 3, P4417R0's p4 and p6): the owning result of a structure with an array member goes to an owning local, which the callee that takes the block receives; an owning member beside an array member is stored from an owning call and destroyed through an owning local
// Pre-registered from record 0064 item 15 and the precedents strict_owner_p4.c and strict_owner_p6.c, before any run: verifies under --strict-owner in both modes (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct B {
  int a[4];
  int *p CERASTES_OWNING;
};

CERASTES_OWNING int *make_int(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

CERASTES_OWNING struct S *make(void)
{
  CERASTES_OWNING struct S *a = (struct S *)calloc(1, sizeof(struct S));
  return a;
}

void take(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
{
  free(p);
}

void b_destroy(struct B *b)
    CERASTES_REQUIRES(b == NULL || cerastes_owns_block(b))
{
  if (b != NULL) {
    free(b->p);
    free(b);
  }
}

int use_owner(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING struct S *p = make();
  if (p != NULL) {
    p->a[1] = 3;
    take(p);
  }
  return 0;
}

int through_field(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct B *b = (struct B *)calloc(1, sizeof(struct B));
  if (b == NULL) {
    return -1;
  }
  b->a[0] = 1;
  b->p = make_int();
  b_destroy(b);
  return 0;
}
