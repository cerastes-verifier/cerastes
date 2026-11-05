// expect: error invalid-free @ L41
// mode: c23, c++20
// twin: tests/memory/heap_pick_held_member.c
// Pins: 0047 item 6 and 0050 item 5 (audit round 3): the block of the member h.b is freed twice on the path where the result is h.a
// Pre-registered from audit round 3's heldmember program with one more free(h.b): exactly one invalid-free at the second free(h.b); on the path c != 0 the result is h.a, the first free(h.b) is the block's only free and the second is a double free; on c == 0 the result is h.b and the else branch frees h.a, so that path is clean
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
    free(h.b);  // <- here
  } else {
    free(h.a);
  }
  return 0;
}
