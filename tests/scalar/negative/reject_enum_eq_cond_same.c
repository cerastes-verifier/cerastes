// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_cond.c
// Pins: 0023 item 7 (Q36): c == (x ? RED : GREEN), the other operand an enumeration value too, is rejected in both modes
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(enum color c, int x)
{
  return c == (x ? RED : GREEN);
}
