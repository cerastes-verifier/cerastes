// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0023 item 1: results at the ends of the range: (-1) ^ INT_MAX is INT_MIN and ~INT_MIN is INT_MAX, no overflow
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
    CERASTES_ENSURES(r, r == -2147483647 - 1)
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
    CERASTES_ENSURES(r, r == -2)
{
  return or_ab(-2147483647 - 1, 2147483646);
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
