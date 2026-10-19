// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 8: right shift of a negative value is arithmetic (floor)
#include <cerastes.h>

int half_floor(int a)
    CERASTES_REQUIRES(a == -7)
    CERASTES_ENSURES(r, r == -4)
{
  return a >> 1;
}

int shr_var(int a, int n)
    CERASTES_REQUIRES(n >= 0 && n <= 31)
{
  return a >> n;
}
