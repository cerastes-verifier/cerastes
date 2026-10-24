// expect: error postcondition @ L37
// mode: c23, c++20
// twin: tests/scalar/bit_or.c
// Pins: 0023 item 1: 12 | 10 is 14, not 15
#include <cerastes.h>

int or_int(int a, int b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

unsigned or_uns(unsigned a, unsigned b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

// Setting a bit with a constant mask.
unsigned or_set_bit(unsigned u)
    CERASTES_ENSURES(r, r == (u | 4u))
{
  return u | 4u;
}

int or_commutes(int a, int b)
    CERASTES_ENSURES(r, r == 1)
{
  int p = a | b;
  int q = b | a;
  return p == q;
}

int or_twelve_ten(void)
    CERASTES_ENSURES(r, r == 15)
{
  return or_int(12, 10);
}

unsigned or_bytes(void)
    CERASTES_ENSURES(r, r == 255u)
{
  return or_uns(240u, 15u);
}

int or_top_low(void)
    CERASTES_ENSURES(r, r == 1073741825)
{
  return or_int(1073741824, 1);
}

unsigned or_all_ones(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return or_uns(4294967294u, 1u);
}

unsigned or_set_bit_value(void)
    CERASTES_ENSURES(r, r == 12u)
{
  return or_set_bit(8u);
}

unsigned or_set_bit_again(void)
    CERASTES_ENSURES(r, r == 4u)
{
  return or_set_bit(4u);
}

unsigned or_set_bit_zero(void)
    CERASTES_ENSURES(r, r == 4u)
{
  return or_set_bit(0u);
}
