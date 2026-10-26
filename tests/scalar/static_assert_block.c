// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 2 (replaces 0012 item 3 for the skip): block-scope static assertions are checked like file-scope ones
#include <cerastes.h>

int widths(int x)
    CERASTES_REQUIRES(x <= 2147483643)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)127 == 127, "a cast that fits");
  static_assert((1 << 30) == 1073741824, "a shift that fits");
  return x + (int)sizeof(int);
}

int use(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return widths(2147483643);
}
