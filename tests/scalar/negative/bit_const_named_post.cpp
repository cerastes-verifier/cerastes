// expect: error postcondition @ L14
// twin: tests/scalar/bit_const_named.cpp
// Pins: 0023 item 6: ALL is LO | HI = 255, not 254
#include <cerastes.h>

const unsigned LO = 15;
const unsigned HI = 240;
const unsigned ALL = LO | HI;
constexpr int BIT = 1 << 3;

unsigned all_value()
    CERASTES_ENSURES(r, r == 254u)
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
