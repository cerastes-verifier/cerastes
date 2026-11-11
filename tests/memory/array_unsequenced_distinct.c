// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 9: a[i] = a[j]++ under i != j: the unsequenced row (addresses differ) holds; no compiler warns on it
// Pre-registered from record 0059 item 9: verifies
#include <cerastes.h>

int shift(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 3 && 0 <= j && j < 3 && i != j)
    CERASTES_ENSURES(r, 2 <= r && r <= 4)
{
  int a[3] = {1, 2, 3};
  a[i] = a[j]++;
  return a[j];
}
