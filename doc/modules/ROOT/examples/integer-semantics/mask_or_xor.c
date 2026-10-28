// expect: verified
// expect: native-ok
// mode: c23, c++20
// A mask by a constant of the form 2^k - 1 sets (|) or flips (^) the low k bits, and the claims about it are decided for every operand.
// tag::source[]
#include <cerastes.h>

int set_low_byte(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x - x % 256 + 255)
{
  return x | 255;
}

int flip_low_byte(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == (x - x % 256) + (255 - x % 256))
{
  return x ^ 255;
}

unsigned long long set_low_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v - v % 4294967296 + 4294967295)
{
  return v | 0xFFFFFFFFu;
}
// end::source[]
