// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/loop_invariant_reads_unowned.c
// Pins: 0040 item 5 (was 0038 items 3 and 6, audit round 2, S1): p is reassigned, so *q is no automatic place and the invariant reads it without a written atom: permission at the invariant
#include <cerastes.h>
#include <stddef.h>

int unowned_read(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 0 && 0 <= n && n <= 10)
{
  int *q = p;
  p = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (i == 0 || *q == 0))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
  return q == NULL;
}
