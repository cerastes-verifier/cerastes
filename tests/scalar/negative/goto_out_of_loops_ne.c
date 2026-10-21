// expect: error invariant-preserved @ L18
// mode: c23, c++20
// twin: tests/scalar/goto_out_of_loops.c
// Pins: 0015 item 2: the target-0 invariant of the inner loop of the positive notices a wrong search test
// The test is i * j != target, so for target 0 the iteration i == 0, j == 0 does not find the pair and moves on.
#include <cerastes.h>

int first_product(int n, int target)
    CERASTES_REQUIRES(0 <= n && n <= 100 && 0 <= target)
    CERASTES_ENSURES(r, (r == -1 || (0 <= r && r < n)) && (target != 0 || n <= 0 || r == 0))
{
  int found = -1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && found == -1 && (target != 0 || i == 0))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n && found == -1 && (target != 0 || j == 0))
    CERASTES_DECREASES(n - j)
    while (j < n) {
      if (i * j != target) {
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
