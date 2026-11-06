// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/owning_loop_swap.c
// Pins: 0052 item 3: the same swap through a plain temporary: when p is overwritten its old block is held only by t, which is not an owner, so the reassignment row fails at p = q
// Pre-registered from record 0052 item 3: leak at p = q
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
    int *t = p;
    p = q;  // <- here
    q = t;
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
