// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/abs_value.c
// Pins: 0010 item 3: -a is checked on the branch where a < 0
// Without the requires, a = INT_MIN takes the negating branch.
#include <cerastes.h>

int absolute(int a)
    CERASTES_ENSURES(r, r >= 0)
{
  return a < 0 ? -a : a;
}
