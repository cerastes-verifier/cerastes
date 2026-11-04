// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *make_cell(int v)
    CERASTES_ENSURES(r, r == NULL || (cerastes_owns_block(r) && *r == v))
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return NULL;
  }
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
  int v = *p;
  drop_cell(p);
  return v == 7 ? 0 : 1;
}
// end::source[]
