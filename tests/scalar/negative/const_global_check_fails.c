// expect: error check @ L20
// mode: c23, c++20
// twin: tests/scalar/const_global_check.c
// Pins: 0017 item 3: x is LIMIT (100), so a check of 101 fails
#include <cerastes.h>

const int LIMIT = 100;

int dbl(int x)
    CERASTES_REQUIRES(0 <= x && x <= LIMIT)
    CERASTES_ENSURES(r, r == 2 * x)
{
  return 2 * x;
}

int at_limit(int x)
    CERASTES_REQUIRES(x == LIMIT)
    CERASTES_ENSURES(r, r == 200)
{
  CERASTES_CHECK(x == 101);
  return dbl(x);
}

int use(void)
    CERASTES_ENSURES(r, r == 200)
{
  int a = dbl(0);
  int b = at_limit(LIMIT);
  return a + b;
}
