// expect: rejected cerastes-shift_operand_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R2: `a >> b + c` (-Wshift-op-parentheses in clang, -Wparentheses in gcc; signed or unsigned, `>>` too)
#include <cerastes.h>

int test(int a, int b, int c, unsigned u, unsigned v, unsigned w)
{
  return a >> b + c;
}
