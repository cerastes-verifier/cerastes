// expect: rejected cerastes-constant_logical_operand @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R9: as the initializer of a local
#include <cerastes.h>

int test(int x, int y)
{
  int r = x || 2;
  return r;
}
