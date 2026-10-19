// expect: verified
// expect: native-ok
// A constant is checked as written: -1 converted to unsigned is modular.
// tag::source[]
#include <cerastes.h>

unsigned all_ones(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  unsigned u = -1;
  return u;
}
// end::source[]
