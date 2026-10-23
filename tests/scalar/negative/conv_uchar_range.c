// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: 257 does not fit unsigned char: a conversion error at the return, never 1
#include <cerastes.h>

unsigned char to_uc(int x)
    CERASTES_REQUIRES(x == 257)
{
  return x;
}
