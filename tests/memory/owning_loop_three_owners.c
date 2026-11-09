// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 3 (the cost, O(n^2) pair clauses): three owning locals rotated through an owning temporary in one loop; every pair of carriers has a guard and an exclusivity clause. The point is the time (S39: 2-4 s for two owners): if this passes 120 s tell the orchestrator before touching the expectation
// Pre-registered from record 0054 item 3 ("A test with three owners in one loop is timed in Part B"): verifies in both modes (derived from the rules for owning_loop_swap.c, not from a spike)
#include <cerastes.h>
#include <stdlib.h>

int rotate(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return -1;
  }
  CERASTES_OWNING int *c = (int *)malloc(sizeof(int));
  if (c == NULL) {
    free(a);
    free(b);
    return -1;
  }
  *a = 0;
  *b = 1;
  *c = 2;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    CERASTES_OWNING int *t = a;
    a = b;
    b = c;
    c = t;
    i += 1;
  }
  free(a);
  free(b);
  free(c);
  return 0;
}
