// expect: rejected cerastes-value_not_used @ L11
// expect: frontend-warning assign_where_compare_meant @ L11
// mode: c23, c++20
// twin: tests/scalar/discarded_and_assign_const.c
// Pins: 0020 item 5: the same with an increment as the left operand
#include <cerastes.h>

int f(int x, int y)
    CERASTES_REQUIRES(x < 2147483647)
{
  x++ && (y = 0);
  return x + y;
}
