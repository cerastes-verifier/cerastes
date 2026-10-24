// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/bit_compound.c
// Pins: 0023 item 3: u &= ~FLAG with an int constant FLAG converts the negative int ~FLAG to unsigned
#include <cerastes.h>

const int FLAG = 4;

unsigned clear_flag(unsigned u)
{
  u &= ~FLAG;
  return u;
}
