// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: a case value is encoded by its expression (shift, sum and cast checked)
#include <cerastes.h>

int pick(int x)
    CERASTES_ENSURES(r, (x == 8 && r == 1) || (x == 3 && r == 2) || (x == 5 && r == 3) || (x != 8 && x != 3 && x != 5 && r == 0))
{
  switch (x) {
  case 1 << 3:
    return 1;
  case 2 + 1:
    return 2;
  case (signed char)5:
    return 3;
  default:
    return 0;
  }
}
