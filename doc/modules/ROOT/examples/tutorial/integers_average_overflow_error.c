// expect: error overflow @ L12
// mode: c23, c++20
// a + b can exceed INT_MAX, so the addition overflows.
// Derived from integers_average.c.
// tag::source[]
#include <cerastes.h>

int average(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= b)
    CERASTES_ENSURES(r, a <= r && r <= b)
{
  return (a + b) / 2;
}
// end::source[]
