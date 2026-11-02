// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: x has a value when it is passed.
// tag::source[]
#include <cerastes.h>

void set(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 1);

int use_set(void)
{
  int x = 0;
  set(&x);
  return x;
}
// end::source[]
