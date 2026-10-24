// expect: error postcondition @ L34
// mode: c23, c++20
// twin: tests/scalar/bit_min_result.c
// Pins: 0023 item 1: (-1) ^ INT_MAX is INT_MIN, not INT_MIN + 1
#include <cerastes.h>

int and_ab(int a, int b)
    CERASTES_ENSURES(r, r == (a & b))
{
  return a & b;
}

int or_ab(int a, int b)
    CERASTES_ENSURES(r, r == (a | b))
{
  return a | b;
}

int xor_ab(int a, int b)
    CERASTES_ENSURES(r, r == (a ^ b))
{
  return a ^ b;
}

int not_a(int a)
    CERASTES_ENSURES(r, r == -1 - a)
{
  return ~a;
}

int xor_to_min(void)
    CERASTES_ENSURES(r, r == -2147483647)
{
  return xor_ab(-1, 2147483647);
}

int not_min(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return not_a(-2147483647 - 1);
}

int not_max(void)
    CERASTES_ENSURES(r, r == -2147483647 - 1)
{
  return not_a(2147483647);
}

int or_min_max(void)
    CERASTES_ENSURES(r, r == -1)
{
  return or_ab(-2147483647 - 1, 2147483647);
}

int and_min_all(void)
    CERASTES_ENSURES(r, r == -2147483647 - 1)
{
  return and_ab(-2147483647 - 1, -1);
}

int xor_min_all(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return xor_ab(-2147483647 - 1, -1);
}

// The same results from constants written in the expression.
int xor_direct(void)
    CERASTES_ENSURES(r, r == -2147483647 - 1)
{
  return -1 ^ 2147483647;
}

int not_direct(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return ~(-2147483647 - 1);
}
