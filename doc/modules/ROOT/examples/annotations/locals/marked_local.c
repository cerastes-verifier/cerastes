// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer local marked ref_to_uninit receives malloc's result; without --strict-init the marker changes no verdict.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int f(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 3;
  int v = *p;
  free(p);
  return v;
}
// end::source[]
