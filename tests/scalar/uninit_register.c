// expect: verified
// expect: native-ok
// c-only: C++17 removed the register storage class
// Pins: 0019 item 2: a register local without an initializer is read only after its assignment (C)
#include <cerastes.h>

int reg(int a)
    CERASTES_ENSURES(r, r == a)
{
  register int x;
  x = a;
  return x;
}
