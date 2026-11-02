// expect: verified
// expect: native-ok
// mode: c23, c++20
// An output parameter: init receives an object without a value and must give it one; the caller reads x afterwards.
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
  int x;
  init(&x);
  return x;
}
// end::source[]
