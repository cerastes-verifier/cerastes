// expect: rejected cerastes-logical_operand_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R1: `a || b && c || d` (-Wlogical-op-parentheses, -Wparentheses: all four)
#include <cerastes.h>

int test(int a, int b, int c, int d)
{
  return a || b && c || d;
}
