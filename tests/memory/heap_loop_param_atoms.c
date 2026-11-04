// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: a pin on verification cost (S1 skips a parameter copy and changes no outcome): the loop and heap rules it exercises have their twins in heap_loop_param_block.c and the d2a loop tests
// Pins: 0047 item 10's follow-up (re-audit S1, M4.5d2v): three owns parameters and two reads parameters of a loop, in a file that also allocates (heapy): a parameter that has an atom place and is never assigned is not copied into a heap place, so the splits do not multiply (t28 timed out at 186 s)
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
