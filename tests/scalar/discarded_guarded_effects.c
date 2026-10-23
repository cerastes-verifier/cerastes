// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 5: a discarded ?: whose arm is x++ is accepted (every compiler is silent)
#include <cerastes.h>

int guarded_incr(int c, int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == (c != 0 ? x + 1 : x))
{
  c ? x++ : 0;
  return x;
}
