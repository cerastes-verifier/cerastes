// expect: error conversion @ L11
// mode: c23, c++20
// twin: tests/scalar/assign_value_unsigned.c
// Pins: 0020 item 2 and 0022: the value of (b = a) converts a to unsigned char with a check: 256 does not fit
#include <cerastes.h>

int wrap(int a)
    CERASTES_REQUIRES(0 <= a)
{
  unsigned char b;
  int t = (b = a) + 1;
  return t + b;
}
