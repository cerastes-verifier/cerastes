// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 item 7: k at its extremes in 32 bits: the mask 0 (k = 0), INT_MAX (k = N - 1 signed), UINT_MAX (k = N unsigned) and INT_MIN (the complement of INT_MAX)
#include <limits.h>
#include <cerastes.h>

int and_zero(int x)
    CERASTES_ENSURES(r, r == 0)
{
  return x & 0;
}

int and_int_max(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == x) || (x < 0 && r == x + 2147483648))
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
