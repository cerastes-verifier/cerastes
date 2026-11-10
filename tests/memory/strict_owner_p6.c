// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-owner
// Pins: 0058 item 3 (p6): every allowed argument of a call whose parameter takes a block without returning it: an owning local, an owning global, a leaf of a struct local (s.data), a leaf of a by-value struct parameter, a field through a pointer (p->data) and through a pointer to a pointer ((*pp)->data), a parameter of the enclosing function whose own contract takes its block, and a p4 call (free(make()))
// Pre-registered from record 0058 item 3: verifies in both modes under --strict-owner
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct account {
  int balance;
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

CERASTES_OWNING struct account *make_account(void)
{
  CERASTES_OWNING struct account *a = (struct account *)malloc(sizeof(struct account));
  if (a != NULL) {
    a->balance = 0;
  }
  return a;
}

void take(struct account *a)
    CERASTES_REQUIRES(cerastes_owns_block(a))
{
  free(a);
}

void clear_inner(struct buf **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  free((*pp)->data);
  (*pp)->data = NULL;
}

int value_parameter(struct buf s)
    CERASTES_REQUIRES(s.data == NULL || cerastes_owns_block(s.data))
    CERASTES_ENSURES(r, r == 0)
{
  free(s.data);
  return 0;
}

int owning_local(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = make();
  free(v);
  return 0;
}

int struct_leaf(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf s = { make(), 0 };
  free(s.data);
  return 0;
}

int through_pointer(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  b->data = make();
  free(b->data);
  free(b);
  return 0;
}

int consuming_parameter(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING struct account *a = make_account();
  if (a != NULL) {
    take(a);
  }
  return 0;
}

int free_the_call(void)
    CERASTES_ENSURES(r, r == 0)
{
  free(make());
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
