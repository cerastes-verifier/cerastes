// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/uchar_promote.c
// Pins: 0010 item 6 and 0022: the sum 254 + 1 is 255, never 256
#include <cerastes.h>

int sum_uc(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

unsigned char wrap_uc(unsigned char a)
    CERASTES_REQUIRES(a == 254)
    CERASTES_ENSURES(r, r == 256)
{
  return a + 1;
}
