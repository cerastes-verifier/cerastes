// expect: error overflow @ L15
// mode: c23, c++20
// twin: tests/scalar/static_assert_block.c
// Pins: 0024 item 2: static_assert_block.c with a wrapping assertion inside `if (0) { }`; clang and gcc accept it
#include <cerastes.h>

int widths(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)127 == 127, "a cast that fits");
  static_assert((1 << 30) == 1073741824, "a shift that fits");
  if (0) {
    static_assert(65536u * 65536u == 0u, "dead but checked");
  }
  return x + (int)sizeof(int);
}

int use(void)
    CERASTES_ENSURES(r, r == 99 + 4)
{
  return widths(99);
}
