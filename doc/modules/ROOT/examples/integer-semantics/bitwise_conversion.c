// expect: error conversion @ L10
// mode: c23, c++20
// ~7 is the negative int -8: converting it to unsigned for the mask does not fit, so cerastes reports a conversion. The fix is ~7u.
// tag::source[]
#include <cerastes.h>

unsigned clear_low_bits(unsigned u)
    CERASTES_ENSURES(r, r == (u & ~7u))
{
  return u & ~7;
}
// end::source[]
