// expect: rejected cerastes-int_in_truth_context @ L10
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R7: `*` under `!`, parenthesized
#include <cerastes.h>
#include <stdbool.h>

int test(int a, int b, int c, int x, unsigned u, unsigned v)
{
  return !(a * b);
}
