// expect: error overflow @ L14
// mode: c23, c++20
// twin: tests/scalar/uninit_compound.c
// Pins: 0019 item 2 and 0022: the third increment of a + 3 passes UINT_MAX
#include <cerastes.h>

unsigned int bump(unsigned int a)
    CERASTES_REQUIRES(a == 4294967293u)
{
  unsigned int x;
  x = a;
  x += 1u;
  x++;
  ++x;
  return x;
}
