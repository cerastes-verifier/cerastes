// expect: error invalid-free @ L34
// expect: error invalid-free @ L35
// mode: c23, c++20
// twin: tests/memory/heap_call_results_many.c
// Pins: 0047 item 6 and 0050 item 5 (audit round 3, the result split): the call's result is one of the two blocks, so after free(r) one of free(q) and free(p) frees a block twice, on its own path each: invalid-free at both frees
// Pre-registered from audit round 3 (a2r/s4h_double.c: invalid-free at its second and third frees): invalid-free at free(q) and at free(p)
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
  free(q);  // <- here
  free(p);  // <- here
  return 0;
}
