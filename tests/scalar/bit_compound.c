// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 3: &=, |= and ^= (statement and value forms) compute in the usual conversions and convert back with a check
#include <cerastes.h>

int and_assign(int x, int m)
    CERASTES_ENSURES(r, r == (x & m))
{
  x &= m;
  return x;
}

unsigned and_assign_uns(unsigned x, unsigned m)
    CERASTES_ENSURES(r, r == (x & m))
{
  x &= m;
  return x;
}

int or_assign(int x, int m)
    CERASTES_ENSURES(r, r == (x | m))
{
  x |= m;
  return x;
}

int xor_assign(int x, int m)
    CERASTES_ENSURES(r, r == (x ^ m))
{
  x ^= m;
  return x;
}

// The value of a compound assignment is the stored value.
int or_value(int x)
    CERASTES_ENSURES(r, r == (x | 1))
{
  int y = (x |= 1);
  return y;
}

int and_value(int x)
    CERASTES_ENSURES(r, r == (x & 255))
{
  int y;
  y = (x &= 255);
  return y;
}

// Exchange without a temporary, for constants.
int xor_swap_constants(void)
    CERASTES_ENSURES(r, r == 93)
{
  int a = 3;
  int b = 9;
  a ^= b;
  b ^= a;
  a ^= b;
  return a * 10 + b;
}

int and_assign_value(void)
    CERASTES_ENSURES(r, r == 5)
{
  return and_assign(13, 7);
}

int or_assign_value(void)
    CERASTES_ENSURES(r, r == 6)
{
  return or_assign(2, 4);
}

int xor_assign_value(void)
    CERASTES_ENSURES(r, r == 6)
{
  return xor_assign(12, 10);
}
