// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: the value is read, then the allocation is freed before the return.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int stash(int a)
    CERASTES_ENSURES(r, r == 0 || r == a)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 0;
  }
  *p = a;
  int v = *p;
  free(p);
  return v;
}
// end::source[]
