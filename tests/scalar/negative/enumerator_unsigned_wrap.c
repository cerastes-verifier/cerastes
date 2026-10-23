// expect: error overflow @ L8
// mode: c23, c++20
// twin: tests/scalar/enumerator_unsigned_expr.c
// Pins: 0022 items 2 and 11: 0u - 1u in an enumerator initializer is an overflow at the enumerator (exit 1, never 3), not UINT_MAX
#include <cerastes.h>

enum {
  A = (0u - 1u) > 5u
};

int a_value(void)
    CERASTES_ENSURES(r, r == 1)
{
  return A;
}
