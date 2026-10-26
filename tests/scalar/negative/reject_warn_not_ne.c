// expect: rejected cerastes-logical_not_comparison @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R3: `!x != y` (-Wlogical-not-parentheses: all four)
#include <cerastes.h>

int test(int x, int y)
{
  return !x != y;
}
