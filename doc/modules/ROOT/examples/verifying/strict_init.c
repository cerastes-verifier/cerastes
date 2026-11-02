// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// With --strict-init every local without an initializer is marked and only the address of a marked local is passed to a marked parameter.
// tag::source[]
#include <cerastes.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

int use_init(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_UNINIT int x;
  init(&x);
  return x;
}
// end::source[]
