// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 10: ?: has the value of the chosen branch
#include <cerastes.h>

int max2(int a, int b)
    CERASTES_ENSURES(r, r >= a && r >= b)
{
  return a > b ? a : b;
}
