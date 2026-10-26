// expect: rejected cerastes-int_in_truth_context @ L12
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R7: an enumerator under `!` (gcc C++)
#include <cerastes.h>
#include <stdbool.h>

enum e7 { Z7 = 0, ONE7 = 1, TWO7 = 2 };

int test(int c)
{
  return !TWO7;
}
