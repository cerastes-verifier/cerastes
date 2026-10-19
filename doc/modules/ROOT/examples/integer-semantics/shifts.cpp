// expect: verified
// expect: native-ok
// Right shift of a negative value is arithmetic; left shift of an unsigned value wraps.
// tag::source[]
#include <cerastes.h>

int halve(int a)
    CERASTES_REQUIRES(a == -8)
    CERASTES_ENSURES(r, r == -4)
{
  return a >> 1;
}

unsigned top_bit(unsigned x)
    CERASTES_REQUIRES(x == 3u)
    CERASTES_ENSURES(r, r == 2147483648u)
{
  return x << 31;
}
// end::source[]
