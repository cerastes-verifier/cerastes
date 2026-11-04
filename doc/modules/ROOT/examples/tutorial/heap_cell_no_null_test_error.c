// expect: error null-dereference @ L13
// mode: c23, c++20
// malloc may return NULL and the test is gone, so *p = v may dereference null.
// Derived from heap_cell.c.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *make_cell(int v)
    CERASTES_ENSURES(r, r == NULL || (cerastes_owns_block(r) && *r == v))
{
  int *p = (int *)malloc(sizeof(int));
  *p = v;
  return p;
}

void drop_cell(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  free(p);
}

int main(void)
{
  int *p = make_cell(7);
  if (p == NULL) {
    return 1;
  }
  CERASTES_CHECK(*p == 7);
  drop_cell(p);
  return 0;
}
// end::source[]
