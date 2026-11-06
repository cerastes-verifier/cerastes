// expect: error leak @ L18
// mode: c23, c++20
// One path returns without freeing: the owner still holds its block there.
// Derived from heap_owner.c.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int round_trip(int v)
    CERASTES_ENSURES(r, r == v || r == -1 || r == -2)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = v;
  if (v < 0) {
    return -2;
  }
  int w = *p;
  free(p);
  return w;
}
// end::source[]
