// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1: | is exact on signed and unsigned operands and carries no obligation
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
    CERASTES_ENSURES(r, r == 14)
{
  return or_int(12, 10);
}

unsigned or_bytes(void)
    CERASTES_ENSURES(r, r == 6u)
{
  return or_uns(2u, 4u);
}

int or_top_low(void)
    CERASTES_ENSURES(r, r == 1073741826)
{
  return or_int(1073741824, 2);
}

unsigned or_halves(void)
    CERASTES_ENSURES(r, r == 4294901760u)
{
  return or_uns(4278190080u, 16711680u);
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
