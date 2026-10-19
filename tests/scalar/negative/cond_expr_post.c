// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/cond_expr.c
// Pins: 0010 item 10: the false branch returns a, which may be below b
#include <cerastes.h>

int max2(int a, int b)
    CERASTES_ENSURES(r, r >= a && r >= b)
{
  return a > b ? a : a;
}
