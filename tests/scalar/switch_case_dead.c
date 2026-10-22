// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0017 item 5: case values are checked at the entry, so a switch reached on some inputs only still verifies
// The switch is reached only when x > 1000; its case values are expressions that are well defined.
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, r == (x <= 1000 ? 0 : x == 1024 ? 1 : x == 2000 ? 2 : 3))
{
  if (x <= 1000) {
    return 0;
  }
  switch (x) {
  case 1 << 10:
    return 1;
  case 2 * 1000:
    return 2;
  default:
    return 3;
  }
}
