// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 item 2: a goto leaves nested loops; nothing is checked on that path (as for break)
// The invariants and measures hold on every iteration that does not jump; found is -1 after the loops
// run out, and i after the jump.
#include <cerastes.h>

int first_product(int n, int target)
    CERASTES_REQUIRES(0 <= n && n <= 100 && 0 <= target)
    CERASTES_ENSURES(r, r == -1 || (0 <= r && r < n))
{
  int found = -1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && found == -1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n && found == -1)
    CERASTES_DECREASES(n - j)
    while (j < n) {
      if (i * j == target) {
        found = i;
        goto out;
      }
      j += 1;
    }
    i += 1;
  }
out:
  return found;
}
