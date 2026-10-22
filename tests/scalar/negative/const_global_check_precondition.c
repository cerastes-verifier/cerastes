// expect: error precondition @ L22
// mode: c23, c++20
// twin: tests/scalar/const_global_check.c
// Pins: 0017 item 3: x + 1 is 101, beyond the callee's requires
// at_limit's ensures is changed to what dbl(x + 1) returns, so the error is only the precondition.
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
    CERASTES_ENSURES(r, r == 202)
{
  CERASTES_CHECK(x == 100);
  return dbl(x + 1);
}

int use(void)
    CERASTES_ENSURES(r, r == 202)
{
  int a = dbl(0);
  int b = at_limit(LIMIT);
  return a + b;
}
