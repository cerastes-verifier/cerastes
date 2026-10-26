// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/scalar/static_assert_closed.c
// Pins: 0024 item 6: the type rule runs on the operands of sizeof inside assertions too
#include <cerastes.h>

static_assert(sizeof(char[4]) == 4, "rejected");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
