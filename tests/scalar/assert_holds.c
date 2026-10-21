// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 6: assert(e) from <assert.h> is checked: it holds on every path
// The assertion is a check (category check) with the macro's text; the program is verified as if NDEBUG were off.
#include <assert.h>
#include <cerastes.h>

int next(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x + 1;
  assert(y > 0 && y <= 101);
  return y;
}
