// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: a <<= n shifts in the promoted type of a alone; a long n does not widen it
#include <cerastes.h>

int shl_assign(int a, long n)
    CERASTES_REQUIRES(a == 1 && n == 4)
    CERASTES_ENSURES(r, r == 16)
{
  a <<= n;
  return a;
}
