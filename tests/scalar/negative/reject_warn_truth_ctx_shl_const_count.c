// expect: rejected cerastes-int_in_truth_context @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R7: a constant shift count does not exempt it
#include <cerastes.h>
#include <stdbool.h>

int test(int a, int b, int c, int x, unsigned u, unsigned v)
{
  if (a << 1) {
    return 1;
  }
  return 0;
}
