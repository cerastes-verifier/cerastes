// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 3 and 6: each block entry allocates its array anew and each closing brace takes it back
// Pre-registered from record 0059 item 6: verifies
#include <cerastes.h>

int two_blocks(void)
    CERASTES_ENSURES(r, r == 3)
{
  int s = 0;
  {
    int a[2] = {1, 2};
    s = a[0] + a[1];
  }
  {
    int b[2] = {0};
    s = s + b[0];
  }
  return s;
}
