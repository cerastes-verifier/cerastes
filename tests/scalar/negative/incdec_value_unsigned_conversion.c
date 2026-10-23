// expect: error conversion @ L9
// mode: c23, c++20
// twin: tests/scalar/incdec_value_unsigned.c
// Pins: 0020 item 1 and 0022: c++ at 255 computes 256 in int; converting it back to unsigned char does not fit
#include <cerastes.h>

int wrap(unsigned char c)
{
  int a = c++;
  return a + c;
}
