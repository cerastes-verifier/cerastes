// expect: error permission @ L34
// mode: c23, c++20
// twin: tests/memory/struct_owning_field.c
// Pins: 0055 item 3 (twin of the flipped struct_owning_field.c): the borrower frees the structure the owning field points to and returns without setting it to NULL, "returns while owning field n->p does not hold its block"
// Pre-registered from record 0055 item 3: permission at the closing brace of clear (the return row of the clause)
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
}  // <- here

int ident(int a)
{
  return a;
}
