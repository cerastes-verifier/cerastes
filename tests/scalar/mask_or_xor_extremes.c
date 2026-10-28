// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: k at its extremes in 32 bits: x | 0 and x ^ 0 (k = 0), INT_MAX (k = N - 1 signed), UINT_MAX (k = N unsigned) and INT_MIN (the complement of INT_MAX)
#include <limits.h>
#include <cerastes.h>

int or_zero(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x | 0;
}

int xor_zero(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x ^ 0;
}

int or_int_max(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == 2147483647) || (x < 0 && r == -1))
{
  return x | INT_MAX;
}

int xor_int_max(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == 2147483647LL - x) || (x < 0 && r == -1LL - x - 2147483648LL))
{
  return x ^ INT_MAX;
}

int or_int_min(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == x - 2147483648LL) || (x < 0 && r == x))
{
  return x | INT_MIN;
}

int xor_int_min(int x)
    CERASTES_ENSURES(r, (x >= 0 && r == x - 2147483648LL) || (x < 0 && r == x + 2147483648LL))
{
  return x ^ INT_MIN;
}

unsigned or_uint_max(unsigned u)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return u | UINT_MAX;
}

unsigned xor_uint_max(unsigned u)
    CERASTES_ENSURES(r, r == 4294967295u - u)
{
  return u ^ UINT_MAX;
}
