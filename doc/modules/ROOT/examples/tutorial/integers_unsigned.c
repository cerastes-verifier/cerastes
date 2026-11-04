// expect: verified
// expect: native-ok
// mode: c23, c++20
// Unsigned arithmetic is checked too: the subtraction cannot go below zero.
// tag::source[]
#include <cerastes.h>

unsigned difference(unsigned a, unsigned b)
    CERASTES_REQUIRES(a >= b)
    CERASTES_ENSURES(r, r == a - b)
{
  return a - b;
}
// end::source[]
