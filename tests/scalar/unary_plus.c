// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 10: unary plus is the identity (after promotion)
#include <cerastes.h>

int same(int a)
    CERASTES_ENSURES(r, r == a)
{
  return +a;
}
