// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 9: when every path returns, the closing brace is unreachable (no missing-return)
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, (x > 0 && r == 1) || (x <= 0 && r == 2))
{
  if (x > 0) {
    return 1;
  } else {
    return 2;
  }
}
