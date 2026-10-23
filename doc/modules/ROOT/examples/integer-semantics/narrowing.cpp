// expect: verified
// expect: native-ok
// A conversion to a signed type is checked, and so is a conversion to an unsigned type.
// tag::source[]
#include <cerastes.h>

signed char small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 127)
    CERASTES_ENSURES(r, r == x)
{
  return (signed char)x;
}

unsigned char low_byte(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == x % 256)
{
  return (unsigned char)(x % 256);
}
// end::source[]
