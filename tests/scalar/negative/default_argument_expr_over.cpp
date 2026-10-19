// expect: error overflow @ L17
// twin: tests/scalar/default_argument_expr.cpp
// Pins: 0012 item 6: the default 100000 * 100000 overflows int, reported at the call
// The note points at the default argument in the declaration.
#include <cerastes.h>

int pick(int a, int b = 100000 * 100000)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == a)
{
  (void)b;
  return a;
}

int use(void)
{
  return pick(1);
}
