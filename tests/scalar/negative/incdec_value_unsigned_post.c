// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/incdec_value_unsigned.c
// Pins: 0020 item 1: c++ wraps 255 to 0
#include <cerastes.h>

int wrap(unsigned char c)
    CERASTES_ENSURES(r, r == c + c + 1)
{
  int a = c++;
  return a + c;
}
