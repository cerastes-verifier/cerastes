// expect: error postcondition @ L24
// twin: tests/scalar/default_argument.cpp
// Pins: 0012 item 6: the two calls at the ends of the callee's range add up to 2000, not 1999
#include <cerastes.h>

int add_default(int a, int b = 10)
    CERASTES_REQUIRES(a >= 0 && a <= 1000 && b >= 0 && b <= 1000)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

int use(void)
    CERASTES_ENSURES(r, r == 11)
{
  return add_default(1);
}

int use_edges(void)
    CERASTES_ENSURES(r, r == 1999)
{
  int low = add_default(0, 1000);
  int high = add_default(1000, 0);
  return low + high;
}
