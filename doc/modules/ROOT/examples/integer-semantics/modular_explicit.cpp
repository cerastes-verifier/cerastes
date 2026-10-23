// expect: verified
// expect: native-ok
// A modular result is written explicitly: reduce in the same type, then convert, so the conversion is exact.
// tag::source[]
#include <cerastes.h>

unsigned char low_byte(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == x % 256)
{
  return (unsigned char)(x % 256);
}
// end::source[]
