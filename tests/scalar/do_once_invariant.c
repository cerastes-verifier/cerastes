// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 5: a written invariant that holds on entry is accepted on do ... while (0)
#include <cerastes.h>

int add_one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  int y = x;
  CERASTES_INVARIANT(y == x)
  do {
    y += 1;
  } while (0);
  return y;
}
