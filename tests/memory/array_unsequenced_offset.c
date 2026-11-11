// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 9: a[i + 1] = a[j]++ under i + 1 != j (0 <= i < 2, 0 <= j < 3): the two elements differ, so the unsequenced row holds; no compiler warns on it
// Pre-registered from record 0059 item 9: verifies
#include <cerastes.h>

int offset(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3 && i + 1 != j)
    CERASTES_ENSURES(r, 2 <= r && r <= 4)
{
  int a[3] = {1, 2, 3};
  a[i + 1] = a[j]++;
  return a[j];
}
