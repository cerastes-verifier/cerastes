// expect: error postcondition @ L13
// expect: frontend-warning assign_where_compare_meant @ L12
// mode: c23, c++20
// twin: tests/scalar/discarded_and_assign_warn.c
// Pins: 0025 item 4: an error test lists the same warning; the postcondition fails for a == 0 (y stays 0, the contract says 1)
#include <cerastes.h>

int and_assign(int a)
    CERASTES_ENSURES(r, r == (a != 0 ? 2 : 1))
{
  int y = 0;
  a && (y = 2);
  return y;
}
