// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_extremes.c
// Pins: 0026 item 7: for a negative x, x & INT_MAX is x + 2^31, not x + 2^31 - 1 (x == -1 refutes it)
#include <limits.h>
#include <cerastes.h>

int and_zero(int x)
    CERASTES_ENSURES(r, r == 0)
{
  return x & 0;
}

int and_int_max(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == x) || (x < 0 && r == x + 2147483647))
{
  return x & INT_MAX;
}

unsigned and_uint_max(unsigned u)
    CERASTES_ENSURES(r, r == u)
{
  return u & UINT_MAX;
}

int and_int_min(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == 0) || (x < 0 && r == INT_MIN))
{
  return x & INT_MIN;
}
