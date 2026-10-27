// expect: verified
// expect: native-ok
// mode: c23, c++20
// A mask by a constant of the form 2^k - 1 is a remainder, and by its complement it clears the low k bits.
// tag::source[]
#include <cerastes.h>

unsigned long long low_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v % 4294967296)
{
  return v & 0xFFFFFFFFu;
}

int clear_low_nibble(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 16)
{
  return x & ~15;
}
// end::source[]
