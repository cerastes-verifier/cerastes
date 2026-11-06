// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning local holds an allocation and frees it on every path.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int round_trip(int v)
    CERASTES_ENSURES(r, r == v || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = v;
  int w = *p;
  free(p);
  return w;
}
// end::source[]
