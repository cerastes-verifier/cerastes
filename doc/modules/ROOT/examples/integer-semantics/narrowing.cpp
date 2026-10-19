// expect: verified
// expect: native-ok
// A conversion to a signed type is checked; a conversion to an unsigned type wraps.
// tag::source[]
#include <cerastes.h>

signed char small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 127)
    CERASTES_ENSURES(r, r == x)
{
  return (signed char)x;
}

unsigned char low_byte(int x)
    CERASTES_ENSURES(r, r >= 0)
{
  return (unsigned char)x;
}
// end::source[]
