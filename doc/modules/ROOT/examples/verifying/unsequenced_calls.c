// expect: rejected cerastes-unsequenced_calls @ L21
// mode: c23, c++20
// Each division depends on a different call, and C may run the two calls in either order. If f never returned, the division by g(y) could be reached or hidden depending on the order, so cerastes refuses it. Name the results first: int a = f(x); int b = g(y); return 100 / a + 100 / b;
// tag::source[]
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int g(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int bad(int x, int y)
{
  return 100 / f(x) + 100 / g(y);
}
// end::source[]
