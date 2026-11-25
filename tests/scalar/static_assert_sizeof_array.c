// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 14 (flip A of M4.5e1e1, was reject_static_assert_sizeof_array.c): the type rule runs on the operands of sizeof inside assertions too, and the assertion holds; unchanged, line for line (twin negative/static_assert_sizeof_array_wrap.c)
#include <cerastes.h>

static_assert(sizeof(char[4]) == 4, "rejected");

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
