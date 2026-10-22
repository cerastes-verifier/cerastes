// expect: error uninitialized @ L10
// c-only: C++17 removed the register storage class
// twin: tests/scalar/uninit_register.c
// Pins: 0019 item 2: a register local read before its assignment (C)
#include <cerastes.h>

int reg(int a)
{
  register int x;
  int y = x;
  x = a;
  return y;
}
