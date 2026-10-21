// expect: error precondition @ L25
// twin: tests/scalar/default_argument_expr.cpp
// Pins: 0012 item 6: the caller's requires z >= -1 lets -1 through, which the callee's a >= 0 refuses (the last function of the positive)
#include <cerastes.h>

int pick(int a, int b = 1000 * 1000)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r == a)
{
  (void)b;
  return a;
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  return pick(1);
}

// Zero for both parameters: a stronger requires of `pick` fails here.
int use_zero(int z)
    CERASTES_REQUIRES(z >= -1)
    CERASTES_ENSURES(r, r == z)
{
  return pick(z, z);
}
