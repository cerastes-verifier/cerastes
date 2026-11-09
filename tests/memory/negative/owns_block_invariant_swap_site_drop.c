// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_swap.c
// Pins: 0054 item 4 (audit round 2, Part B, blocking hole, probe q1b): a local pointer p with a written site (p == NULL || owns_block(p)) that the loop sets to NULL: the block it held is dropped, and the invariant still holds (p is NULL); the loop does not allocate, so nothing replaces the dropped block: the iteration ends owning an allocation the loop does not carry: leak at the loop body's closing brace (the back-edge row)
// Pre-registered from audit round 2 (Part B): leak at the loop body's closing brace; the probe VERIFIED when this was written (the fix has landed and the line is confirmed)
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(p == NULL || cerastes_owns_block(p))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    p = NULL;
    i += 1;
  }  // <- here
  if (p != NULL) {
    free(p);
  }
  return 0;
}
