// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4: &a[N] is formed in value position (0 <= i <= N), never accessed
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int one_past(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  [[maybe_unused]] int *e = &a[3];
  return a[0];
}
