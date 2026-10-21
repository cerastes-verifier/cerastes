// expect: error check @ L30
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_loops.c
// Pins: 0015 item 2: after the label only what every incoming path gives is known
// found == -1 holds on the normal exit of the loops, not on the goto path (found == i).
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
  CERASTES_CHECK(found == -1);
  return found;
}
