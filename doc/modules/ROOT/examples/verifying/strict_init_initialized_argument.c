// expect: rejected cerastes-uninit_ref_initialized @ L15
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: an initialized object passed to a marked parameter.
// tag::source[]
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0);

int f(void)
{
  int x = 5;
  init(&x);
  return x;
}
// end::source[]
