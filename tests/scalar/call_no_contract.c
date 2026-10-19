// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0011 item 5: a function with a body and no contract promises only its types' ranges
#include <cerastes.h>

int ident(int x)
{
  return x;
}

long use(int a)
    CERASTES_ENSURES(r, r >= -2147483647L - 1 && r <= 2147483647L)
{
  return ident(a);
}
