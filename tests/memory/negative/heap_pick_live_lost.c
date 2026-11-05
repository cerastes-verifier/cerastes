// expect: error invalid-free @ L32
// mode: c23, c++20
// twin: tests/memory/heap_pick_live.c
// Pins: 0047 item 6 (audit round 2 S4): pick keeps a share of each liveness cell (no `ensures` live), so the free of its result finds a pointer that may still be in use elsewhere
// Pre-registered from il-analyst's S4 fix (a2/g2_plat_pick_body.c): exactly one invalid-free at free(r) ("frees `r` while a pointer to it may still be in use elsewhere"); the path ends there

#include <stdlib.h>
#include <cerastes.h>

int *pick(int *a, int *b, int c)
    CERASTES_REQUIRES(cerastes_live(a) && cerastes_live(b))
    CERASTES_ENSURES(r, (c != 0 && r == a) || (c == 0 && r == b))
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
  free(r);  // <- here
  if (c != 0) {
    free(q);
  } else {
    free(p);
  }
  return 0;
}
