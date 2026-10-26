// expect: error conversion @ L16
// twin: tests/scalar/char_literal_default_arg.cpp
// Pins: 0024 item 5: char_literal_default_arg.cpp with b = '\xff': reported at the call, like default_argument_expr_over.cpp (0012 item 6); the explicit call is fine
#include <cerastes.h>

int pick(int a, char b = '\xff')
    CERASTES_REQUIRES(a >= 0 && a <= 100)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

int use(void)
    CERASTES_ENSURES(r, r == 127)
{
  return pick(0);
}

int use_explicit(void)
    CERASTES_ENSURES(r, r == 11)
{
  return pick(1, 10);
}
