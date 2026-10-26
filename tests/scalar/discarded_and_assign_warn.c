// expect: verified
// expect: native-ok
// expect: frontend-warning assign_where_compare_meant @ L13
// expect: frontend-warning assign_where_compare_meant @ L21
// mode: c23, c++20
// Pins: 0025 item 4 (question 31): a store in the right operand of a discarded && or ||; every compiler accepts it silently (P48), EDG prints its #187 (assign_where_compare_meant), which the test lists
#include <cerastes.h>

int and_assign(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? 2 : 0))
{
  int y = 0;
  a && (y = 2);
  return y;
}

int or_assign(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? 0 : 1))
{
  int y = 0;
  a || (y = 1);
  return y;
}
