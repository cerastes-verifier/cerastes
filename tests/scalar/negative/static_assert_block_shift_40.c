// expect: error shift @ L14
// mode: c23
// twin: tests/scalar/static_assert_block.c
// c-only: EDG refuses (1 << 40) == 0 in C++ (#28, not a constant), so there is no C++ run; clang and gcc refuse it natively too
// Pins: 0024 item 2: static_assert_block.c with a shift past the width in C23
#include <cerastes.h>

int widths(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)127 == 127, "a cast that fits");
  _Static_assert((1 << 40) == 0, "a shift past the width");
  return x + (int)sizeof(int);
}

int use(void)
    CERASTES_ENSURES(r, r == 99 + 4)
{
  return widths(99);
}
