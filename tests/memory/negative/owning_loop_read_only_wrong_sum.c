// expect: error invariant-preserved @ L18
// mode: c23, c++20
// twin: tests/memory/owning_loop_read_only.c
// Pins: 0052 item 3 Loops (audit 3 S1): the same loop that adds one more than the invariant says: invariant-preserved at the invariant
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 S1: invariant-preserved at the invariant
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return 1;
  }
  *v = 5;
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && s == 5 * i)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += *v + 1;
    i += 1;
  }
  free(v);
  return s;
}
