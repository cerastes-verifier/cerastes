// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-owner
// Pins: 0058 item 3 (p4): every allowed position of a call whose contract gives its result's block: the initializer of an owning local, the value assigned to an owning local, to an owning global and to a static local, a store through an owning field (p->data = make()), an owning field's leaf in a struct initializer (the compound literal is c23 only: strict_owner_p4_compound_literal.c), the operand of return in an owning function, and the argument of free (a parameter whose contract takes the block); every one of them also verifies without the option (the option only rejects more)
// Pre-registered from record 0058 item 3: verifies in both modes under --strict-owner
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING static int *gp = NULL;

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

CERASTES_OWNING int *make_again(void)
{
  return make();
}

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int local_and_assign(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = make();
  free(v);
  v = make_again();
  free(v);
  free(make());
  return 0;
}

int through_field(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = make();
  buf_destroy(b);
  return 0;
}

int struct_initializer(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf s = { make(), 0 };
  free(s.data);
  return 0;
}

int static_local(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING static int *cache = NULL;
  if (cache == NULL) {
    cache = (int *)malloc(sizeof(int));
    if (cache == NULL) {
      return -1;
    }
  }
  *cache = 1;
  return 0;
}

int main(void)
{
  gp = (int *)malloc(sizeof(int));
  if (gp == NULL) {
    return 1;
  }
  *gp = 1;
  free(gp);
  return 0;
}
