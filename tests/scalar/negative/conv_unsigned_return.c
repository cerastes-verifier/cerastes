// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned_sites.c
// Pins: 0022 item 1: returning -1 from an unsigned function is a conversion error at the return
#include <cerastes.h>

unsigned from_return(int x)
    CERASTES_REQUIRES(x == -1)
{
  return x;
}
