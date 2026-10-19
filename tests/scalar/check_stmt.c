// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 3: check is a statement; it reads the current values
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x > 0 && x < 100)
{
  int y = x + 1;
  CERASTES_CHECK(y > 1 && y <= 100);
  return y;
}
