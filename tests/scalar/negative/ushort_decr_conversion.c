// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/incdec_small_unsigned.c
// Pins: 0022 item 1: us-- at 0 computes -1 in int; converting it back does not fit unsigned short
#include <cerastes.h>

unsigned short dec_us(unsigned short v)
    CERASTES_REQUIRES(v == 0)
    CERASTES_ENSURES(r, r == 65535)
{
  v--;
  return v;
}
