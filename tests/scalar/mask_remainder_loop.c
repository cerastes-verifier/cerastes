// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 items 7 and 8: a loop that clears the low two bits of a signed variable with the complement ~3 in each iteration: the value only goes down, so it stays at most n
#include <cerastes.h>

int mask_down(int n, int k)
    CERASTES_REQUIRES(1 <= k && k <= 10)
    CERASTES_ENSURES(r, r <= n)
{
  int x = n;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= k && x <= n)
  CERASTES_DECREASES(k - i)
  while (i < k) {
    x = x & ~3;
    i = i + 1;
  }
  return x;
}
