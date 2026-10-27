// expect: rejected cerastes-int_in_truth_context @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R7: `?:` with constant arms 2 and 3 in a condition (clang by default, gcc with -Wall)
#include <cerastes.h>
#include <stdbool.h>

int test(int a, int b, int c, int x, unsigned u, unsigned v)
{
  if (c ? 2 : 3) {
    return 1;
  }
  return 0;
}
