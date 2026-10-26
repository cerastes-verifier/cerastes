// expect: error conversion @ L6
// twin: tests/scalar/static_assert_file.c
// Pins: 0024 items 2 and 5: static_assert('\xff') (C++, one argument): the char -1 converted to bool by the front end; the literal's own conversion is the error
#include <cerastes.h>

static_assert('\xff');
static_assert(65535u * 65537u == 4294967295u, "an unsigned product that fits");
static_assert((signed char)127 == 127, "a cast that fits");
static_assert((1 << 30) == 1073741824, "a shift that fits");
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
