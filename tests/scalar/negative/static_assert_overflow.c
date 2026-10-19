// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/scalar/static_assert_ok.c
// Pins: 0010 item 3: the same function without its requires overflows; static_assert changes nothing
#include <cerastes.h>

int bump(int x)
    CERASTES_ENSURES(r, r >= x)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  return x + 1;
}
