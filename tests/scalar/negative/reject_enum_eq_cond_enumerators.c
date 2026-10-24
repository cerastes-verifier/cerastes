// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/scalar/enum_eq_truth_cond.c
// Pins: 0023 item 7 (Q36): (x ? RED : GREEN) has the enumeration's type in C++ and is rejected there (0018 item 8); C types it int, and the same shape is rejected in C too, whatever the other operand is
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int f(int x, int y)
{
  return (x ? RED : GREEN) == y;
}
