// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): two blocks and a pointer parameter the loop resets: the places of the two token cells and of the parameter's copy are all distinct or equal as the splits say; the parameter t has `t == NULL || live(t)` in its requires because a pointer use (here the comparison before the loop; after the loop t is only a copy the loop reset) checks liveness (0028 item 3a)
#include <cerastes.h>
#include <stdlib.h>

int f(int *t, int n)
    CERASTES_REQUIRES(0 <= n && n <= 100 && (t == NULL || cerastes_live(t)))
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 0;
  int i = 0, was_null = (t == NULL);
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    t = NULL;
    i += 1;
  }
  free(p);
  free(q);
  return was_null;
}
