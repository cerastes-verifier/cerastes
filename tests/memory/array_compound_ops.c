// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4: +=, ++ and -- on elements reach INT_MAX without overflow
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

#include <limits.h>

int bump(void)
    CERASTES_ENSURES(r, r == INT_MAX)
{
  int a[2] = {INT_MAX - 2, 0};
  a[0] += 1;
  a[0]++;
  a[1]--;
  CERASTES_CHECK(a[1] == -1);
  return a[0];
}
