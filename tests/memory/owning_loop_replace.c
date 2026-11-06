// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3 (amends 0038 item 3 and 0047 item 10): the verified twin of heap_loop_replace_false_alarm.c: p and q are owning locals declared outside the loop, and the loop frees q and allocates another into it: the loop place of q is its current value
// Pre-registered from record 0052 item 3 (the loop rule): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int mixed(int n)
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
  *q = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    free(q);
    q = (int *)malloc(sizeof(int));
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
