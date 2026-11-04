// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/heap_loop_carry_and_free.c
// Pins: 0047 item 10's follow-up (re-audit S2, M4.5d2v): p is carried to the next iteration but q's new block is not: an iteration ends owning an allocation the loop does not carry (the back-edge exemption is exact): leak at the loop body's closing brace
// Pre-registered from .claude/plans/m4-5d2v-allocator.md Part 1 (t34): leak at the closing brace of the loop body, one error
#include <cerastes.h>
#include <stdlib.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    q = (int *)malloc(sizeof(int));
    if (q != NULL) {
      *q = i;
    }
    i += 1;
  }  // <- here
  free(p);
  return 0;
}
