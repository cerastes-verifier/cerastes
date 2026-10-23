// expect: verified
// expect: native-ok
// A constant is checked as written: 4000000000 fits unsigned int, so the conversion needs no check.
// tag::source[]
#include <cerastes.h>

unsigned big(void)
    CERASTES_ENSURES(r, r == 4000000000u)
{
  unsigned u = 4000000000;
  return u;
}
// end::source[]
