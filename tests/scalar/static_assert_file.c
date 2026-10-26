// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 2: file-scope static assertions are read from the file scope's source sequence list and checked in the prologue of every method
#include <cerastes.h>

static_assert(sizeof(int) == 4);
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
    CERASTES_REQUIRES(x <= 2147483646)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}
