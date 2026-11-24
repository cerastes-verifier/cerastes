// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 3 (P81 (e)): a block-scope static_assert in a read pure body is checked as in code (0024 item 2): holding assertions change nothing
// Pre-registered from record 0062 (Part B), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int widen(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == x + 4)
{
  static_assert(sizeof(int) == 4, "int is 32 bits");
  static_assert((signed char)127 == 127, "a cast that fits");
  return x + (int)sizeof(int);
}

int use(int y)
    CERASTES_REQUIRES(0 <= y && y <= 1000)
{
  CERASTES_CHECK(widen(y) == y + 4);
  return y;
}
