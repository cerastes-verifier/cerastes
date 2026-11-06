// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// With --strict-init a local that receives malloc's result is marked ref_to_uninit; calloc's result has a value, so its local needs no marker.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int fresh(void)
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

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *q = (int *)calloc(1, sizeof(int));
  if (q == NULL) {
    return -1;
  }
  int v = *q;
  free(q);
  return v;
}
// end::source[]
