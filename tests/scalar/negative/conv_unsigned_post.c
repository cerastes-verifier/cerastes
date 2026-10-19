// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/conv_unsigned.c
// Pins: 0010 item 5: 257 converts to 1 in unsigned char, not 257
#include <cerastes.h>

unsigned char to_uc(int x)
    CERASTES_REQUIRES(x == 257)
    CERASTES_ENSURES(r, r == 257)
{
  return x;
}

unsigned to_u(int x)
    CERASTES_REQUIRES(x == -1)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return x;
}
