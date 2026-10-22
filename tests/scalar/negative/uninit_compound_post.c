// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_compound.c
// Pins: 0019 item 2: the compound assignments read assigned values; the sum is wrong
#include <cerastes.h>

unsigned int bump(unsigned int a)
    CERASTES_REQUIRES(a <= 100)
    CERASTES_ENSURES(r, r == a + 3)
{
  unsigned int x;
  x = a;
  x += 1u;
  x++;
  return x;
}
