// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/static_assert_file.c
// Pins: 0024 item 2: static_assert_file.c with an unsigned subtraction that wraps
#include <cerastes.h>

static_assert(sizeof(int) == 4);
static_assert(65535u * 65537u == 4294967295u, "an unsigned product that fits");
static_assert((signed char)127 == 127, "a cast that fits");
static_assert(0u - 1u == 4294967295u, "a subtraction that wraps");
static_assert(-7 / 2 == -3 && -7 % 2 == -1, "division truncates toward zero");

int four(void)
    CERASTES_ENSURES(r, r == 4)
{
  return (int)sizeof(int);
}

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}
