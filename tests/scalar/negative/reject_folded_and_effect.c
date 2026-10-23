// expect: rejected cerastes-unsupported_expression @ L12
// expect: frontend-warning assign_where_compare_meant @ L12
// mode: c23, c++20
// twin: tests/scalar/and_assign.c
// Pins: 0012 item 12 and 0020 item 5 (M4.3a audit note 5): EDG folds `0 && (x = 1)`, so its assignment is never evaluated and cannot be encoded; rejected, with EDG's #187 printed at the assignment
#include <cerastes.h>

int f(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x = a;
  int y = 0 && (x = 1);
  return x + y;
}
