// expect: error shift @ L13
// mode: c23, c++20
// twin: tests/scalar/static_assert_block.c
// Pins: 0024 item 2: static_assert_block.c with 1 << 31: EDG, clang and gcc accept it in both modes (C++20 defines it); the result 2^31 does not fit int, so it is a shift error. (1 << 40) is not usable: EDG's C++ mode refuses it itself, #28)
#include <cerastes.h>

int widths(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)127 == 127, "a cast that fits");
  static_assert((1 << 31) == -2147483647 - 1, "a shift into the sign bit");
  return x + (int)sizeof(int);
}

int use(void)
    CERASTES_ENSURES(r, r == 99 + 4)
{
  return widths(99);
}
