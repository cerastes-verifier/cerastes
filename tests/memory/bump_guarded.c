// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 6, 9: (*p)++ at statement level, with the overflow check of the operator
#include <cerastes.h>

int bump_guarded(int k)
    CERASTES_REQUIRES(0 <= k)
    CERASTES_ENSURES(r, r >= k)
{
  int x = k;
  int *p = &x;
  if (*p < 2147483647) {
    (*p)++;
  }
  return x;
}
