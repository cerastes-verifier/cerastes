// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/scalar/uchar_promote.c
// Pins: 0010 item 6: the sum wraps in unsigned char, never 256
#include <cerastes.h>

int sum_uc(unsigned char a, unsigned char b)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

unsigned char wrap_uc(unsigned char a)
    CERASTES_REQUIRES(a == 255)
    CERASTES_ENSURES(r, r == 256)
{
  return a + 1;
}
