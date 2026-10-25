// expect: error postcondition @ L42
// mode: c23, c++20
// twin: tests/scalar/bit_xor.c
// Pins: 0023 item 1: 12 ^ 10 is 6, not 7
#include <cerastes.h>

int xor_int(int a, int b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

unsigned xor_uns(unsigned a, unsigned b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

int xor_self(int a)
    CERASTES_ENSURES(r, r == 0)
{
  return a ^ a;
}

unsigned xor_self_uns(unsigned a)
    CERASTES_ENSURES(r, r == 0u)
{
  return a ^ a;
}

int xor_commutes(int a, int b)
    CERASTES_ENSURES(r, r == 1)
{
  int p = a ^ b;
  int q = b ^ a;
  return p == q;
}

int xor_twelve_ten(void)
    CERASTES_ENSURES(r, r == 7)
{
  return xor_int(12, 10);
}

unsigned xor_low_byte(void)
    CERASTES_ENSURES(r, r == 4294967040u)
{
  return xor_uns(4294967295u, 255u);
}

int xor_equal(void)
    CERASTES_ENSURES(r, r == 0)
{
  return xor_int(7, 7);
}

// Applying the same operand twice gives the value back, here for constants.
int xor_twice_value(void)
    CERASTES_ENSURES(r, r == 12)
{
  return xor_int(xor_int(12, 10), 10);
}
