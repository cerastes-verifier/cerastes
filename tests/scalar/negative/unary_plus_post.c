// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/unary_plus.c
// Pins: 0010 item 10: +a is a, not -a
#include <cerastes.h>

int same(int a)
    CERASTES_REQUIRES(a >= -2147483647)
    CERASTES_ENSURES(r, r == -a)
{
  return +a;
}
