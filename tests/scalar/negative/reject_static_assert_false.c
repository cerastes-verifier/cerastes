// expect: rejected static_assert @ L10
// mode: c23, c++20
// twin: tests/scalar/static_assert_ok.c
// Pins: 0012 item 3: a false static_assert is EDG's error
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
{
  static_assert(sizeof(int) == 8, "int is not 64 bits");
  return x + 1;
}
