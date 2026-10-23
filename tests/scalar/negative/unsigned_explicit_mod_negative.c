// expect: error conversion @ L9
// mode: c23, c++20
// twin: tests/scalar/unsigned_explicit_mod.c
// Pins: 0022 item 7: without x >= 0, x % 256 may be negative and (unsigned char) of it does not fit
#include <cerastes.h>

unsigned char low_byte(int x)
{
  return (unsigned char)(x % 256);
}
