// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 6 and 0050 item 5 (audit round 3, the result split): the two blocks are held only through the members of a structure local, and the call's result is one of them: the argument split sees the members' values, so the block the result names is freed once
// Pre-registered from audit round 3 (a2r/s4c2_heldmember.c): verifies in both modes
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

struct H {
  int *a;
  int *b;
};

int f(int c)
{
  struct H h;
  h.a = (int *)malloc(sizeof(int));
  if (h.a == NULL) {
    return -1;
  }
  h.b = (int *)malloc(sizeof(int));
  if (h.b == NULL) {
    free(h.a);
    return -1;
  }
  int *r = pick(h.a, h.b, c);
  free(r);
  if (c != 0) {
    free(h.b);
  } else {
    free(h.a);
  }
  return 0;
}
