// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1: & is exact on signed and unsigned operands and carries no obligation
#include <cerastes.h>

int and_int(int a, int b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

unsigned and_uns(unsigned a, unsigned b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

// A constant mask bounds the result, whatever the operand (and whatever its sign).
unsigned and_byte(unsigned u)
    CERASTES_ENSURES(r, r == (u & 255u))
{
  return u & 255u;
}

int and_byte_int(int a)
    CERASTES_ENSURES(r, r == (a & 255))
{
  return a & 255;
}

int and_byte_range(int a)
    CERASTES_ENSURES(r, 0 <= r && r <= 255)
{
  return and_byte_int(a);
}

// Testing the low bit is the remainder modulo two.
int and_parity(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r == x % 2)
{
  return x & 1;
}

int and_zero(int a)
    CERASTES_ENSURES(r, r == 0)
{
  return a & 0;
}

int and_parity_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  return and_parity(0);
}

int and_commutes(int a, int b)
    CERASTES_ENSURES(r, r == 1)
{
  int p = a & b;
  int q = b & a;
  return p == q;
}

// Exact values at the callers: 1101 & 0111, the low byte of an all-ones word, two large operands.
int and_thirteen_seven(void)
    CERASTES_ENSURES(r, r == 5)
{
  return and_int(13, 7);
}

unsigned and_low_byte(void)
    CERASTES_ENSURES(r, r == 255u)
{
  return and_uns(4294967295u, 255u);
}

int and_large(void)
    CERASTES_ENSURES(r, r == 1073741821)
{
  return and_int(2147483645, 1073741821);
}

int and_parity_odd(void)
    CERASTES_ENSURES(r, r == 1)
{
  return and_parity(2147483647);
}
