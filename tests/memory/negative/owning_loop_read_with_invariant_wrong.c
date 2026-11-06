// expect: error invariant-entry @ L18
// mode: c23, c++20
// twin: tests/memory/owning_loop_read_with_invariant.c
// Pins: 0052 item 3 Loops (audit 3 S1): the user invariant claims a value the entry does not meet (*v == 6 where *v == 5): invariant-entry at the clause
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 S1: invariant-entry at the clause
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
  CERASTES_INVARIANT(0 <= i && i <= 3 && s == 5 * i && v != NULL && *v == 6)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += *v;
    i += 1;
  }
  free(v);
  return s;
}
