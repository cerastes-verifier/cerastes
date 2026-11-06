// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// With --strict-init a function marked ref_to_uninit returns a marked local, a null pointer or a call of another such function.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *raw(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  return p;
}

CERASTES_REF_TO_UNINIT int *raw_or_null(int fail)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  if (fail != 0) {
    return NULL;
  }
  return raw();
}

int use(void)
    CERASTES_ENSURES(r, r == 4 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = raw_or_null(0);
  if (p == NULL) {
    return -1;
  }
  *p = 4;
  int v = *p;
  free(p);
  return v;
}
// end::source[]
