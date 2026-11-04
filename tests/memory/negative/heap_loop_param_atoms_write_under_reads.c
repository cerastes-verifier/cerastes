// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/heap_loop_param_atoms.c
// Pins: 0047 item 10's follow-up (re-audit S1, M4.5d2v): the parameters with atom places that the loop does not assign are not copied into heap places, and a write to *c under reads(*c) in the loop is still `permission` (skipping the copy drops no check)
// Pre-registered from audit round 1 S4: permission at the write under reads
#include <cerastes.h>
#include <stdlib.h>

void many(int *a, int *b, int *c, int *d, int *e, int n)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b) && cerastes_reads(*c) && cerastes_reads(*d) && cerastes_owns(*e) && 0 <= n && n <= 10)
    CERASTES_ENSURES(cerastes_owns(*a) && cerastes_owns(*b) && cerastes_reads(*c) && cerastes_reads(*d) && cerastes_owns(*e))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *a = *c;
    *b = *d;
    *e = *c;
    *c = i;  // <- here
    i += 1;
  }
}

int heapy(void)
{
  int *h = (int *)malloc(sizeof(int));
  if (h == NULL) {
    return -1;
  }
  free(h);
  return 0;
}
