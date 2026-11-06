// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning local holds an allocation and frees it on every path: at the end of the block and at the return.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int use_and_free(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  free(p);
  return v;
}

int block_scope(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  {
    CERASTES_OWNING int *q = (int *)malloc(sizeof(int));
    if (q == NULL) {
      return -1;
    }
    *q = c;
    free(q);
  }
  return 0;
}
// end::source[]
