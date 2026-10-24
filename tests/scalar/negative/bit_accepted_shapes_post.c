// expect: error postcondition @ L112
// mode: c23, c++20
// twin: tests/scalar/bit_accepted_shapes.c
// Pins: 0023 item 5: !0 & !0 is 1, not 2
#include <cerastes.h>
#include <stdbool.h>

bool is_pos(int a)
    CERASTES_ENSURES(r, r == (a > 0))
{
  return a > 0;
}

// !a & !b: the right operand is a truth value.
int both_zero(int a, int b)
    CERASTES_ENSURES(r, r == (!a & !b))
{
  return !a & !b;
}

// (!x) & y: the negation is parenthesized.
int not_x_and_y(int x, int y)
    CERASTES_ENSURES(r, r == ((!x) & y))
{
  return (!x) & y;
}

// x & !y: the negation is the right operand.
int x_and_not_y(int x, int y)
    CERASTES_ENSURES(r, r == (x & !y))
{
  return x & !y;
}

// a & b ? c : d with ordinary operands.
int pick_if_bits(int a, int b, int c, int d)
    CERASTES_ENSURES(r, r == ((a & b) ? c : d))
{
  return a & b ? c : d;
}

// A bool and a call: only one side has a side effect.
int both(bool p, int a)
    CERASTES_ENSURES(r, r == (p & (a > 0)))
{
  return p & is_pos(a);
}

int xor_bools(bool b1, bool b2)
    CERASTES_ENSURES(r, r == (b1 ^ b2))
{
  return b1 ^ b2;
}

int xor_self_assign(int x)
    CERASTES_ENSURES(r, r == 0)
{
  x ^= x;
  return x;
}

// ~uc is a negative int: the mask keeps its low byte, and a negative constant is a plain comparison.
int not_byte(unsigned char uc)
    CERASTES_ENSURES(r, r == (~uc & 0xFF))
{
  return ~uc & 0xFF;
}

int not_is_minus_one(unsigned char uc)
    CERASTES_ENSURES(r, r == (uc == 0))
{
  return ~uc == -1;
}

int or_cmp(int x, int y, int z)
    CERASTES_ENSURES(r, r == (x | (y == z)))
{
  x |= y == z;
  return x;
}

int or_and(int a, int b, int c)
    CERASTES_ENSURES(r, r == (a | (b & c)))
{
  return a | (b & c);
}

int and_all_ones(int x)
    CERASTES_ENSURES(r, r == (x & -1))
{
  return x & -1;
}

int nonzero(int x)
    CERASTES_ENSURES(r, r == ((x | 0) != 0))
{
  if (x | 0) {
    return 1;
  }
  return 0;
}

long not_or_one(int x)
    CERASTES_ENSURES(r, r == (!x | 1L))
{
  return !x | 1L;
}

int both_zero_value(void)
    CERASTES_ENSURES(r, r == 2)
{
  return both_zero(0, 0);
}

int both_zero_one_nonzero(void)
    CERASTES_ENSURES(r, r == 0)
{
  return both_zero(0, 5);
}

int not_x_and_y_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return not_x_and_y(0, 5);
}

int x_and_not_y_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return x_and_not_y(5, 0);
}

int pick_if_bits_taken(void)
    CERASTES_ENSURES(r, r == 10)
{
  return pick_if_bits(1, 1, 10, 20);
}

int pick_if_bits_not_taken(void)
    CERASTES_ENSURES(r, r == 20)
{
  return pick_if_bits(1, 2, 10, 20);
}

int both_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return both(true, 1);
}

int both_zero_argument(void)
    CERASTES_ENSURES(r, r == 0)
{
  return both(true, 0);
}

int xor_bools_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return xor_bools(true, false);
}

int xor_bools_equal(void)
    CERASTES_ENSURES(r, r == 0)
{
  return xor_bools(true, true);
}

int not_byte_zero(void)
    CERASTES_ENSURES(r, r == 255)
{
  return not_byte(0);
}

int not_byte_max(void)
    CERASTES_ENSURES(r, r == 0)
{
  return not_byte(255);
}

int not_byte_fifteen(void)
    CERASTES_ENSURES(r, r == 240)
{
  return not_byte(15);
}

int not_is_minus_one_zero(void)
    CERASTES_ENSURES(r, r == 1)
{
  return not_is_minus_one(0);
}

int not_is_minus_one_one(void)
    CERASTES_ENSURES(r, r == 0)
{
  return not_is_minus_one(1);
}

int or_cmp_equal(void)
    CERASTES_ENSURES(r, r == 5)
{
  return or_cmp(4, 3, 3);
}

int or_and_value(void)
    CERASTES_ENSURES(r, r == 12)
{
  return or_and(8, 12, 5);
}

int and_all_ones_negative(void)
    CERASTES_ENSURES(r, r == -5)
{
  return and_all_ones(-5);
}

int nonzero_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  return nonzero(0);
}

int nonzero_one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return nonzero(1);
}

long not_or_one_zero(void)
    CERASTES_ENSURES(r, r == 1L)
{
  return not_or_one(0);
}

