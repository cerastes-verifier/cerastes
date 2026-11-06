// expect: error leak @ L17
// mode: c23, c++20
// twin: tests/memory/owning_loop_null_entry.c
// Pins: 0052 item 3 (audit B2): the same loop without the free: the owner is NULL at entry but holds a block from the second iteration on, so the reassignment row fails there (the loop invariant carries held_v; a loop that leaks every iteration from a NULL entry is the case the audit found verifying): leak at the assignment
// Pre-registered from record 0052 item 3 (audit round 1, S12): leak at the assignment in the loop body
#include <cerastes.h>
#include <stdlib.h>

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    v = (int *)malloc(sizeof(int));  // <- here
    i += 1;
  }
  free(v);
  return 0;
}
