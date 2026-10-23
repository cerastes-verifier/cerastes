// expect: verified
// expect: native-ok
// Right shift of a negative value is arithmetic; left shift of an unsigned value must fit.
// tag::source[]
#include <cerastes.h>

int halve(int a)
    CERASTES_REQUIRES(-8 <= a && a <= -7)
    CERASTES_ENSURES(r, r == -4)
{
  return a >> 1;
}

unsigned top_bit(unsigned x)
    CERASTES_REQUIRES(x == 1u)
    CERASTES_ENSURES(r, r == 2147483648u)
{
  return x << 31;
}
// end::source[]
