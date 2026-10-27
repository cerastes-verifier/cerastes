// expect: verified
// expect: native-ok
// mode: c23, c++20
// Storing the results of two calls in one expression is accepted: whichever call runs first, x and y get the same values. The sum is still checked for overflow, after both calls.
// tag::source[]
#include <cerastes.h>

int capped(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == (x > 1000 ? 1000 : x));

int small(int y)
    CERASTES_REQUIRES(y >= 0)
    CERASTES_ENSURES(r, r == (y > 500 ? 500 : y));

int both(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == 2 * ((a > 1000 ? 1000 : a) + (b > 500 ? 500 : b)))
{
  int x;
  int y;
  int s = (x = capped(a)) + (y = small(b));
  return s + x + y;
}
// end::source[]
