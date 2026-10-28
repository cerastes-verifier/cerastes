// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 4: a cell declared in a switch body is allocated before the dispatch; each case writes it before it is read
#include <cerastes.h>

int pick(int code)
    CERASTES_ENSURES(r, code != 0 || r == 5)
{
  switch (code) {
    int x;
    int *p;
  case 0:
    p = &x;
    *p = 5;
    return x;
  default:
    p = &x;
    *p = 6;
    CERASTES_CHECK(x == 6);
    return x;
  }
}
