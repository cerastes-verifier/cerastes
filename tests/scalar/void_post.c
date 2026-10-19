// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 9: a void function asserts its ensures at each return; and at the closing brace
#include <cerastes.h>

void guard(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(x >= 0)
{
  if (x == 0) {
    return;
  }
  CERASTES_CHECK(x > 0);
}
