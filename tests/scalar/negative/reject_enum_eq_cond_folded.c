// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_cond.c
// Pins: 0023 item 7 (Q36): a folded (1 ? NB : NC) is found inside the constant's original expression, in both modes
#include <cerastes.h>

enum neg { NA = -1, NB, NC };

int f(enum neg n)
{
  return n == (1 ? NB : NC);
}
