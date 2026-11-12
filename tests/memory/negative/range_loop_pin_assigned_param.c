// expect: error lifetime @ L17
// mode: c23, c++20
// twin: tests/memory/range_loop_search.c
// Pins: PIN of question 237 (0060 item 11, M4.5e2): the function assigns its range parameter (p = &p[1]), so p has no range place, and the loop reads p[i] through the new value inside the range the function holds: the read is not framed by any place of the loop, and is reported as one lifetime error at the read (measured by il-analyst-3; the program is correct: p + 1 stays in p's range). A loop that assigns the parameter and does not read through it verifies (the first version of this pin, p = q, did)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

int f(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n))
{
  int i = 0;
  int z = 0;
  p = &p[1];
  CERASTES_INVARIANT(0 <= i && i <= n - 1 && 0 <= z && z <= i) CERASTES_DECREASES(n - 1 - i) while (i < n - 1) {
    if (p[i] == 0) {
      z++;
    }
    i++;
  }
  return z;
}
