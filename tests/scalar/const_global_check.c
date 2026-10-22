// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0017 item 3: a constant in a callee's requires and in a caller's check has its value
// LIMIT is 100: dbl(LIMIT) meets the precondition and its result is 200.
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
  CERASTES_CHECK(x == 100);
  return dbl(x);
}

int use(void)
    CERASTES_ENSURES(r, r == 200)
{
  int a = dbl(0);
  int b = at_limit(LIMIT);
  return a + b;
}
