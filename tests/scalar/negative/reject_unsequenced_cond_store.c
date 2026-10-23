// expect: rejected cerastes-unsequenced_modification @ L10
// mode: c23, c++20
// twin: tests/scalar/assign_stored_sequenced.c
// Pins: 0020 item 5: the modification of x is unsequenced with another use of x in the same full expression
#include <cerastes.h>

int f(int c, int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  x = (c ? x++ : 0);
  return x;
}
