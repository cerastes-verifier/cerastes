// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 6: an int as the condition of if and as a contract is read as e != 0
#include <cerastes.h>

int nonzero(int x)
    CERASTES_ENSURES(r, r == (x != 0))
{
  if (x) {
    return 1;
  }
  return 0;
}

int one_over(int x)
    CERASTES_REQUIRES(x)
{
  return 100 / x;
}
