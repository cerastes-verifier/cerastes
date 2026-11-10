// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 2 (the flip of reject_struct_owning_field.c, 0043 item 15, 0054 item 10): cerastes::owning on a field in the leading position, whose pointee is a structure without owning fields; the borrower reads through it after a null test and frees its block, setting it to NULL
// Pre-registered from record 0055 items 2, 3: verifies in both modes (the field was EDG's #1835 until M4.5d2b2b)
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int x;
  int y;
};

struct N {
  CERASTES_OWNING struct S *p;
  int v;
};

int get_x(struct N *n)
    CERASTES_REQUIRES(cerastes_owns(*n))
    CERASTES_ENSURES(r, cerastes_owns(*n))
{
  if (n->p == NULL) {
    return n->v;
  }
  return n->p->x;
}

void clear(struct N *n)
    CERASTES_REQUIRES(cerastes_owns(*n))
    CERASTES_ENSURES(cerastes_owns(*n))
{
  free(n->p);
  n->p = NULL;
}

int ident(int a)
{
  return a;
}
