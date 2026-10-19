// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 7: division and remainder truncate toward zero
#include <cerastes.h>

int quot(int a, int b)
    CERASTES_REQUIRES(a == -7 && b == 2)
    CERASTES_ENSURES(r, r == -3)
{
  return a / b;
}

int rem(int a, int b)
    CERASTES_REQUIRES(a == -7 && b == 2)
    CERASTES_ENSURES(r, r == -1)
{
  return a % b;
}
