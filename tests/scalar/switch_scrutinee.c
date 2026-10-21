// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 6: the scrutinee is an expression with its own obligations
#include <cerastes.h>

int is_minus_one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, (x == -1 && r == 1) || (x != -1 && r == 0))
{
  switch (x + 1) {
  case 0:
    return 1;
  default:
    return 0;
  }
}
