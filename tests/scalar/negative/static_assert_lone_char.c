// expect: error conversion @ L8
// mode: c23
// twin: tests/scalar/static_assert_file.c
// c-only: _Static_assert is C11/C23; C++ spells it static_assert (static_assert_lone_char.cpp)
// Pins: 0024 items 2 and 5: _Static_assert('\xff', "") (C): the condition's constant is the character literal -1 (an int), which EDG accepts as true
#include <cerastes.h>

_Static_assert('\xff', "a lone wrapped literal");
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
