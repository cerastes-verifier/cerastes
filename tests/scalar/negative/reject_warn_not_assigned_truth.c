// expect: rejected cerastes-logical_not_comparison @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R3: the right operand is an assignment of a truth value: gcc warns (-Wlogical-not-parentheses), clang and EDG do not; the union rejects it
#include <cerastes.h>

int test(int x, int a, int b, int y)
{
  int t = !x == (y = (a > b));
  return t + y;
}
