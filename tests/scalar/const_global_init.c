// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0017 items 3 and 4 and 0022: an initializer is checked as written (shift, an unsigned product that fits, a cast)
// G is 8, SC is 5 and U is UINT_MAX (65535u * 65537u, exact) in both modes. Constants that name other constants are C++ only
// (const_global_init_named.cpp): C does not accept a const object in a constant expression.
#include <cerastes.h>

const int G = 1 << 3;
const unsigned U = 65535u * 65537u;
const signed char SC = (signed char)5;

int combine(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 13)
{
  CERASTES_CHECK(0 <= x);
  CERASTES_CHECK(x <= 100);
  return x + G + SC;
}

unsigned all_ones(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return U;
}

int use(void)
    CERASTES_ENSURES(r, r == 131)
{
  int a = combine(0);
  int b = combine(100);
  int c = (int)(all_ones() - 4294967290u);
  return a + b + c;
}
