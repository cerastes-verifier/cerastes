// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/scalar/enumerator_unsigned_expr.c
// Pins: 0022 item 2: (5u - 1u) > 3u is 1, not 0
#include <cerastes.h>

enum {
  A = (5u - 1u) > 3u
};

int a_value(void)
    CERASTES_ENSURES(r, r == 0)
{
  return A;
}
