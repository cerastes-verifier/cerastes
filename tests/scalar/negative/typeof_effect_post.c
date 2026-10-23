// expect: error postcondition @ L13
// mode: c23
// c-only: typeof is a C23 keyword; the C++ spelling is decltype, which clang warns about (-Wunevaluated-expression)
// twin: tests/scalar/typeof_effect.c
// Pins: 0020 item 5: typeof(x++) leaves x alone
#include <cerastes.h>

int unevaluated(int x)
    CERASTES_REQUIRES(-1073741824 <= x && x <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * x + 1)
{
  typeof(x++) y = x;
  return y + x;
}
