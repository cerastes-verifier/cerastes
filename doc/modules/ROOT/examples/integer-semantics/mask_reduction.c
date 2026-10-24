// expect: verified
// expect: native-ok
// mode: c23, c++20
// A mask is the explicit modular reduction: it keeps the low eight bits of a non-negative value, and the conversion back to unsigned char is then exact.
// tag::source[]
#include <cerastes.h>

unsigned char low_byte(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x % 256)
{
  return (unsigned char)(x & 255);
}
// end::source[]

unsigned char low_byte_of_1000(void)
    CERASTES_ENSURES(r, r == 232)
{
  return low_byte(1000);
}
