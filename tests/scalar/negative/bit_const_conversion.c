// expect: error conversion @ L7
// mode: c23, c++20
// twin: tests/scalar/bit_const.c
// Pins: 0023 item 6 and 0017 item 4: the initializer 0xF0 | 0x100 is 496, which does not fit unsigned char (one conversion error in the prologue)
#include <cerastes.h>

const unsigned char BYTE = 0xF0 | 0x100;

int byte_value(void)
    CERASTES_ENSURES(r, r == 255)
{
  return BYTE;
}
