// expect: rejected cerastes-comma_without_effect @ L10
// mode: c23, c++20
// twin: tests/scalar/comma_statement.c
// Pins: 0020 item 5: the comma in an arm of a discarded ?: is discarded, and its right operand has no effect
#include <cerastes.h>

int f(int a, int x)
    CERASTES_REQUIRES(0 <= x && x < 1000)
{
  a ? (x++, 1) : 0;
  return x;
}
