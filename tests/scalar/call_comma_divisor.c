// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 item 1: the value of a comma carries its floor, so the divisor of 100 / (validate(y), y) is checked after validate, which ends the program unless y > 0
#include <cerastes.h>

// Returns only when y > 0 (a trusted contract: the callee ends the program otherwise).
int validate(int y)
    CERASTES_ENSURES(r, y > 0);

int ratio(int y)
    CERASTES_ENSURES(r, y <= 0 || r == 100 / y)
{
  return 100 / (validate(y), y);
}
