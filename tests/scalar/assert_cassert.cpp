// expect: verified
// expect: native-ok
// Pins: 0015 item 6: <cassert> in C++20 expands to the same assert
// The macro's expansion in C++ has a bool conversion around the condition of __builtin_expect.
#include <cassert>
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x + 1;
  assert(y > 0 && y <= 101);
  return y;
}
