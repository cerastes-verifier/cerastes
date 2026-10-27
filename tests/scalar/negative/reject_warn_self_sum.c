// expect: rejected cerastes-self_comparison @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: `x + 1 == x + 1` (-Wtautological-compare, clang and gcc) (the identical-expression form: gcc only)
#include <cerastes.h>
#include <stdbool.h>

int test(int x, unsigned ux, bool bx)
{
  return x + 1 == x + 1;
}
