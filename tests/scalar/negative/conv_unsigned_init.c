// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: unsigned u = -1 is a conversion error at the initializer (cerastes does not wrap)
#include <cerastes.h>

unsigned from_init(int x)
    CERASTES_REQUIRES(x == -1)
{
  unsigned u = x;
  return u;
}
