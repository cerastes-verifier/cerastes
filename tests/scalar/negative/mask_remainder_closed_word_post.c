// expect: error postcondition @ L46
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_closed.c
// Pins: 0026 item 9: (2^32 + 5) masked to 32 bits is 5, not 4
#include <cerastes.h>

int low_byte(int x)
    CERASTES_ENSURES(r, r == ((x % 256) + 256) % 256)
{
  return x & 255;
}

int clear_nibble(int x)
    CERASTES_ENSURES(r, r == x - ((x % 16) + 16) % 16)
{
  return x & ~15;
}

unsigned low_word(unsigned long long v)
    CERASTES_ENSURES(r, r == v % 4294967296)
{
  return (unsigned)(v & 4294967295u);
}

int low_byte_of_minus_one(void)
    CERASTES_ENSURES(r, r == 255)
{
  return low_byte(-1);
}

int low_byte_of_minus_129(void)
    CERASTES_ENSURES(r, r == 127)
{
  return low_byte(-129);
}

int clear_nibble_of_minus_one(void)
    CERASTES_ENSURES(r, r == -16)
{
  return clear_nibble(-1);
}

unsigned low_word_of_2_pow_32_plus_5(void)
    CERASTES_ENSURES(r, r == 4u)
{
  return low_word(4294967301ull);
}

// ~0xFFu is the unsigned int 4294967040: in 64 bits it is not a complement of 2^k - 1 (it keeps bits 8 to 31).
long long clear_low_byte_unsigned_const(long long x)
    CERASTES_ENSURES(r, r == (x & ~0xFFu))
{
  return x & ~0xFFu;
}

long long clear_low_byte_of_2_pow_32(void)
    CERASTES_ENSURES(r, r == 0)
{
  return clear_low_byte_unsigned_const(4294967296LL);
}
