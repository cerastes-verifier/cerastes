// expect: rejected cerastes-self_comparison @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: the operator is written outside the macro, its operand comes from one: gcc's C mode warns, so it is rejected in both modes
#include <cerastes.h>

#define ID(e) e

int test(int x)
{
  return ID(x) == x;
}
