// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (the loop rule): the verified twin of heap_loop_swap_false_alarm.c: the swap goes through an owning temporary, so each reassignment finds the old block held by an owner that remains (p = q: the old p is t's; q = t: the old q is p's)
// Pre-registered from record 0052 item 3 (the loop rule): verifies in both modes; derived from the rules, not from a spike: if it times out or fails, ask before changing its expectation
#include <cerastes.h>
#include <stdlib.h>

int swapper(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  CERASTES_OWNING int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    CERASTES_OWNING int *t = p;
    p = q;
    q = t;
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
