// expect: error precondition @ L16
// twin: tests/scalar/default_argument.cpp
// Pins: 0012 item 6: the default -1 breaks the callee's requires, reported at the call
// The note points at the default argument in the declaration.
#include <cerastes.h>

int add_default(int a, int b = -1)
    CERASTES_REQUIRES(a >= 0 && a <= 1000 && b >= 0 && b <= 1000)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

int use(void)
{
  return add_default(1);
}
