// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/scalar/assign_value_unsigned.c
// Pins: 0020 item 1: the value of (b = a) is a modulo 256, not modulo 255
#include <cerastes.h>

int wrap(int a)
    CERASTES_REQUIRES(0 <= a)
    CERASTES_ENSURES(r, r == 2 * (a % 255) + 1)
{
  unsigned char b;
  int t = (b = a) + 1;
  return t + b;
}
