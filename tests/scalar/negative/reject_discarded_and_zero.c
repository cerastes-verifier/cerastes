// expect: rejected cerastes-value_not_used @ L10
// expect: frontend-warning assign_where_compare_meant @ L10
// mode: c23, c++20
// twin: tests/scalar/discarded_and_assign_const.c
// Pins: 0020 item 5: a discarded && whose right operand assigns the constant zero to an int is rejected (GCC and clang warn: the && is false and the assignment skipped), where a nonzero constant or a bool target is not
#include <cerastes.h>

int f(int a, int y)
{
  a && (y = 0);
  return y;
}
