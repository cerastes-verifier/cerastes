// expect: error conversion @ L12
// mode: c23, c++20
// twin: tests/scalar/static_assert_block.c
// Pins: 0024 item 2: static_assert_block.c with (signed char)200
#include <cerastes.h>

int widths(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)200 == -56, "a cast that wraps");
  static_assert((1 << 30) == 1073741824, "a shift that fits");
  return x + (int)sizeof(int);
}

int use(void)
    CERASTES_ENSURES(r, r == 99 + 4)
{
  return widths(99);
}
