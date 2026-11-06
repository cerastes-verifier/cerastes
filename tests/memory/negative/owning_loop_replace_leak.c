// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/owning_loop_replace.c
// Pins: 0052 item 3: the loop allocates into the owning local q without freeing the block it held: the reassignment row fails at the assignment, in user terms (heap_loop_replace_false_alarm.c reports the unannotated program at the back edge)
// Pre-registered from record 0052 item 3: leak at the assignment in the loop body
#include <cerastes.h>
#include <stdlib.h>

int mixed(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  CERASTES_OWNING int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    q = (int *)malloc(sizeof(int));  // <- here
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
