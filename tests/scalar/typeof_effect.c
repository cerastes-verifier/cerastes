// expect: verified
// expect: native-ok
// mode: c23
// c-only: typeof is a C23 keyword; the C++ spelling is decltype, which clang warns about (-Wunevaluated-expression)
// Pins: 0020 item 5: typeof(x++) is never evaluated, so x keeps its value (C23 only)
#include <cerastes.h>

int unevaluated(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * x)
{
  typeof(x++) y = x;
  return y + x;
}
