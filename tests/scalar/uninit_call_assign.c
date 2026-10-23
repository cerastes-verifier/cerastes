// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 2: a call's result assigned to a local gives it a value
#include <cerastes.h>

int ident(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int use_ident(int a)
    CERASTES_ENSURES(r, r == a)
{
  int x;
  x = ident(a);
  return x;
}
