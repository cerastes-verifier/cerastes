// expect: error overflow @ L8
// mode: c23, c++20
// twin: tests/scalar/static_assert_sizeof_array.c
// Pins: 0063 item 14: the type rule runs on the operands of sizeof in an assertion with the bound read as written: 65536u * 65536u wraps (the assertion holds for the front end, which says char[1]); overflow at the bound
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error
#include <cerastes.h>

static_assert(sizeof(char[65536u * 65536u + 1]) == 1, "wraps");  // <- here

int one(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1;
}
