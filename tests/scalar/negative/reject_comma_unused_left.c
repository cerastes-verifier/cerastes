// expect: rejected cerastes-comma_without_effect @ L10
// expect: frontend-warning expr_has_no_effect @ L10
// mode: c23, c++20
// twin: tests/scalar/comma_value.c
// Pins: 0020 item 4: a comma whose left operand has no effect is rejected like -Wunused-value does (EDG's #174 prints on the same comma and is not suppressed)
#include <cerastes.h>

int f(int a, int b)
{
  return (a, b);
}
