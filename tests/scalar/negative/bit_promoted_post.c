// expect: error postcondition @ L87
// mode: c23, c++20
// twin: tests/scalar/bit_promoted.c
// Pins: 0023 item 1: 255 & 15 is 15, not 16
#include <cerastes.h>

int and_uchar(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

int or_uchar(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

int xor_uchar(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

// ~ of an unsigned char is a negative int.
int not_uchar(unsigned char a)
    CERASTES_ENSURES(r, r == -1 - a)
{
  return ~a;
}

int and_schar(signed char a, signed char b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

int xor_schar(signed char a, signed char b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

int or_short(short a, short b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

int and_ushort(unsigned short a, unsigned short b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

// Plain char: the operands are kept non-negative so that the result does not depend on its signedness.
int and_char(char a, char b)
    CERASTES_REQUIRES(0 <= a && 0 <= b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

// A constant mask fits the narrow type it converts back to.
unsigned char low_nibble(unsigned char a)
    CERASTES_ENSURES(r, r == a % 16)
{
  return a & 15;
}

signed char low_bits_signed(signed char a)
    CERASTES_ENSURES(r, r == (a & 15))
{
  return a & 15;
}

unsigned char mask_assign(unsigned char a)
    CERASTES_ENSURES(r, r == (a & 15))
{
  a &= 15;
  return a;
}

int and_uchar_value(void)
    CERASTES_ENSURES(r, r == 16)
{
  return and_uchar(255, 15);
}

int or_uchar_value(void)
    CERASTES_ENSURES(r, r == 255)
{
  return or_uchar(240, 15);
}

int xor_uchar_value(void)
    CERASTES_ENSURES(r, r == 0)
{
  return xor_uchar(255, 255);
}

int not_uchar_zero(void)
    CERASTES_ENSURES(r, r == -1)
{
  return not_uchar(0);
}

int not_uchar_max(void)
    CERASTES_ENSURES(r, r == -256)
{
  return not_uchar(255);
}

int and_schar_value(void)
    CERASTES_ENSURES(r, r == 0)
{
  return and_schar(-128, 127);
}

int and_schar_negative(void)
    CERASTES_ENSURES(r, r == -128)
{
  return and_schar(-1, -128);
}

int xor_schar_value(void)
    CERASTES_ENSURES(r, r == -1)
{
  return xor_schar(-128, 127);
}

int or_short_value(void)
    CERASTES_ENSURES(r, r == -1)
{
  return or_short(-32768, 32767);
}

int and_ushort_value(void)
    CERASTES_ENSURES(r, r == 255)
{
  return and_ushort(65535, 255);
}

unsigned char low_nibble_max(void)
    CERASTES_ENSURES(r, r == 15)
{
  return low_nibble(255);
}

signed char low_bits_minus_one(void)
    CERASTES_ENSURES(r, r == 15)
{
  return low_bits_signed(-1);
}
