// expect: rejected cerastes-self_comparison @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: both operands from macros, the operator written outside (gcc C warns)
#include <cerastes.h>

#define ID(e) e

int test(int x)
{
  return ID(x) == ID(x);
}
