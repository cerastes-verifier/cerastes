// expect: error overflow @ L10
// mode: c23, c++20
// In a contract r is any unsigned value, so r + b may overflow.
// Derived from integers_unsigned.c.
// tag::source[]
#include <cerastes.h>

unsigned difference(unsigned a, unsigned b)
    CERASTES_REQUIRES(a >= b)
    CERASTES_ENSURES(r, r + b == a)
{
  return a - b;
}
// end::source[]
