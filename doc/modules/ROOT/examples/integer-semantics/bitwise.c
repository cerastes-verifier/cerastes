// expect: verified
// expect: native-ok
// mode: c23, c++20
// A mask and a flag test. Bitwise operators compute on the two's complement values and never wrap; a mask that clears bits of an unsigned value is written with an unsigned complement (~7u, not ~7).
// tag::source[]
#include <cerastes.h>

unsigned clear_low_bits(unsigned u)
    CERASTES_ENSURES(r, r == (u & ~7u))
{
  return u & ~7u;
}

int has_flag(int flags)
    CERASTES_ENSURES(r, r == ((flags & 4) != 0))
{
  return (flags & 4) != 0;
}
// end::source[]

unsigned clear_low_bits_of_16(void)
    CERASTES_ENSURES(r, r == 16u)
{
  return clear_low_bits(16u);
}
