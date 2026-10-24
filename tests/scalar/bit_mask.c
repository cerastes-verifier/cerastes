// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1 and 0022 item 7: a mask is the explicit modular reduction ((unsigned char)(x & 255) for any x >= 0)
#include <cerastes.h>

unsigned char low_byte(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x % 256)
{
  return (unsigned char)(x & 255);
}

// The low word of a 64-bit value: the conversion is exact because the mask bounds the result.
unsigned low_word(unsigned long long v)
    CERASTES_ENSURES(r, r == (unsigned)(v & 4294967295u))
{
  return (unsigned)(v & 4294967295u);
}

// Clearing the low four bits with a constant mask (~15 is -16), for any sign.
int clear_low_nibble(int x)
    CERASTES_ENSURES(r, r == (x & ~15))
{
  return x & ~15;
}

unsigned char low_byte_1000(void)
    CERASTES_ENSURES(r, r == 232)
{
  return low_byte(1000);
}

unsigned char low_byte_wraps(void)
    CERASTES_ENSURES(r, r == 0)
{
  return low_byte(256);
}

unsigned char low_byte_max(void)
    CERASTES_ENSURES(r, r == 255)
{
  return low_byte(2147483647);
}

unsigned low_word_edge(void)
    CERASTES_ENSURES(r, r == 0u)
{
  return low_word(4294967296ull);
}

unsigned low_word_above(void)
    CERASTES_ENSURES(r, r == 1u)
{
  return low_word(4294967297ull);
}

unsigned low_word_below(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return low_word(4294967295ull);
}

unsigned low_word_all(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return low_word(18446744073709551615ull);
}

int clear_nibble_max(void)
    CERASTES_ENSURES(r, r == 2147483632)
{
  return clear_low_nibble(2147483647);
}

int clear_nibble_small(void)
    CERASTES_ENSURES(r, r == 0)
{
  return clear_low_nibble(15);
}

int clear_nibble_sixteen(void)
    CERASTES_ENSURES(r, r == 16)
{
  return clear_low_nibble(16);
}

int clear_nibble_negative(void)
    CERASTES_ENSURES(r, r == -16)
{
  return clear_low_nibble(-1);
}
