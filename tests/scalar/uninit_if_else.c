// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 3: both arms of an if assign, so the read after it is defined
#include <cerastes.h>

int sign(int a)
    CERASTES_ENSURES(r, (a > 0 && r == 1) || (a <= 0 && r == -1))
{
  int x;
  if (a > 0) {
    x = 1;
  } else {
    x = -1;
  }
  return x;
}
