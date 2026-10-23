// expect: rejected cerastes-assignment_as_condition @ L11
// mode: c23, c++20
// twin: tests/scalar/assign_condition_parens.c
// Pins: 0020 item 4: an assignment used as a truth value without extra parentheses is rejected like -Wparentheses does
#include <cerastes.h>

int f(int a)
    CERASTES_REQUIRES(0 <= a && a < 1000)
{
  int x = 0;
  if (x = a) {
    return x;
  }
  return 0;
}
