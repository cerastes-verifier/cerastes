// expect: rejected cerastes-comma_without_effect @ L10
// mode: c23, c++20
// twin: tests/scalar/comma_statement.c
// Pins: 0020 item 4: a discarded comma whose right operand has no effect is rejected
#include <cerastes.h>

int f(int a, int b)
{
  int x = 0;
  x = a, b;
  return x;
}
