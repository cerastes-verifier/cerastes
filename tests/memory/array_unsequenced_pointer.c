// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 9: *p = a[i]++ with p = &a[1] under i != 1: a store through a pointer to an element and an increment of another element; no compiler warns on it
// Pre-registered from record 0059 item 9: verifies
#include <cerastes.h>

int through(int i)
    CERASTES_REQUIRES(0 <= i && i < 3 && i != 1)
    CERASTES_ENSURES(r, 2 <= r && r <= 4)
{
  int a[3] = {1, 2, 3};
  int *p = &a[1];
  *p = a[i]++;
  return a[i];
}
