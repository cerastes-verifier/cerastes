// expect: verified
// expect: native-ok
// Pins: 0023 items 1 and 6 (0017): C++ constants may name other constants, in initializers and case values
#include <cerastes.h>

const unsigned LO = 15;
const unsigned HI = 240;
const unsigned ALL = LO | HI;
constexpr int BIT = 1 << 3;

unsigned all_value()
    CERASTES_ENSURES(r, r == 255u)
{
  return ALL;
}

int pick(int x)
    CERASTES_ENSURES(r, (x == 9 && r == 1) || (x == 255 && r == 2) || (x != 9 && x != 255 && r == 0))
{
  switch (x) {
  case BIT | 1:
    return 1;
  case (int)HI ^ 15:
    return 2;
  default:
    return 0;
  }
}
