// expect: rejected cerastes-self_comparison @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: an object-like macro for the operand, the operator outside (gcc C warns)
#include <cerastes.h>

#define X x

int test(int x)
{
  return X == x;
}
