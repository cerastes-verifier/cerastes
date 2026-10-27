// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/scalar/mask_remainder_loop.c
// Pins: 0026 items 7 and 8: with at least one iteration (1 <= k) the claim r < n is still false (n == 0, k == 1 gives r == n); it does not depend on how x & ~3 is encoded (the invariant is only x <= n), so this twin pins the failing postcondition at its line, not the mask
#include <cerastes.h>

int mask_down(int n, int k)
    CERASTES_REQUIRES(1 <= k && k <= 10)
    CERASTES_ENSURES(r, r < n)
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
