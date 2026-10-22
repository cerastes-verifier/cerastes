// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 2: a read in an arm of ?: is guarded by the condition
#include <cerastes.h>

int pick(int c, int a)
    CERASTES_ENSURES(r, (c != 0 && r == a) || (c == 0 && r == 0))
{
  int x;
  if (c != 0) {
    x = a;
  }
  return c != 0 ? x : 0;
}
