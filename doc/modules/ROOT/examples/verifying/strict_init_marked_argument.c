// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// With --strict-init a marked local is passed to a ref_to_uninit parameter; free takes it whatever it holds.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

void fill(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void use(void)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return;
  }
  fill(p);
  free(p);
}
// end::source[]
