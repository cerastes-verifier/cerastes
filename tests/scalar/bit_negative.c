// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1: negative operands have their two's complement meaning (-8 & 13 is 8)
#include <cerastes.h>

int and_neg(int a, int b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

int or_neg(int a, int b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

int xor_neg(int a, int b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

// A constant mask of a negative operand is still a small non-negative number.
int and_byte_negative(int a)
    CERASTES_ENSURES(r, r == (a & 255))
{
  return a & 255;
}

int and_byte_negative_range(int a)
    CERASTES_ENSURES(r, 0 <= r && r <= 255)
{
  return and_byte_negative(a);
}

int xor_commutes_negative(int a, int b)
    CERASTES_ENSURES(r, r == 1)
{
  int p = a ^ b;
  int q = b ^ a;
  return p == q;
}

int and_minus_eight(void)
    CERASTES_ENSURES(r, r == 8)
{
  return and_neg(-8, 13);
}

int or_minus_six(void)
    CERASTES_ENSURES(r, r == -6)
{
  return or_neg(-6, 2);
}

int and_minus_five(void)
    CERASTES_ENSURES(r, r == 3)
{
  return and_neg(-5, 3);
}

int and_byte_of_minus_one(void)
    CERASTES_ENSURES(r, r == 255)
{
  return and_byte_negative(-1);
}

int and_byte_of_minus_256(void)
    CERASTES_ENSURES(r, r == 0)
{
  return and_byte_negative(-256);
}

int xor_minus_one(void)
    CERASTES_ENSURES(r, r == -6)
{
  return xor_neg(-1, 5);
}

int and_two_negatives(void)
    CERASTES_ENSURES(r, r == -7)
{
  return and_neg(-5, -3);
}

int or_two_negatives(void)
    CERASTES_ENSURES(r, r == -2)
{
  return or_neg(-4, -2);
}

int xor_two_negatives(void)
    CERASTES_ENSURES(r, r == 5)
{
  return xor_neg(-8, -3);
}

int and_all_ones_value(void)
    CERASTES_ENSURES(r, r == 12345)
{
  return and_neg(-1, 12345);
}

int not_minus_eight(void)
    CERASTES_ENSURES(r, r == 7)
{
  int a = -8;
  return ~a;
}
