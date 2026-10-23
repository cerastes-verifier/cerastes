// expect: rejected cerastes-unsequenced_modification @ L9
// mode: c23, c++20
// twin: tests/scalar/assign_stored_sequenced.c
// Pins: 0020 item 4: in a stored expression a write of x that no sequence point separates from the store is unsequenced with it
#include <cerastes.h>

int f(int x, int y)
{
  x = (y++, x++);
  return x + y;
}
