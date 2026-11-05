// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 6 (audit round 2 S4, the older exit 3 fixed by the call-result split of heap programs): a function that picks one of two pointers, requires live(a) && live(b) and hands both liveness shares back, leaves the caller able to free the one it got
// Pre-registered from il-analyst's S4 fix (a2/g2_fixed_live.c): verifies in both modes

#include <stdlib.h>
#include <cerastes.h>

int *pick(int *a, int *b, int c)
    CERASTES_REQUIRES(cerastes_live(a) && cerastes_live(b))
    CERASTES_ENSURES(r, (c != 0 && r == a) || (c == 0 && r == b))
    CERASTES_ENSURES(cerastes_live(a) && cerastes_live(b))
{
  if (c != 0) {
    return a;
  }
  return b;
}

int f(int c)
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
  int *r = pick(p, q, c);
  free(r);
  if (c != 0) {
    free(q);
  } else {
    free(p);
  }
  return 0;
}
