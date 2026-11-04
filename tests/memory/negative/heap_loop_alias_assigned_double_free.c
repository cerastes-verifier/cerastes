// expect: error invalid-free @ L27
// mode: c23, c++20
// twin: tests/memory/heap_loop_alias_assigned.c
// Pins: 0047 item 10: the block the loop's variables designate is freed twice
// Pre-registered from the loops branch's positive (record 0047 item 10), written after it: the expectation was verified once on the single file before it was written
#include <cerastes.h>
#include <stdlib.h>

int reassigned(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = p;
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    *q = i;
    i += 1;
  }
  free(p);
  free(p);  // <- here
  return 0;
}
