// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/memory/marked_field_loop_written_invariant_written_first.c
// Pins: 0058 item 4 (audit S5, PINNED FALSE ALARM): a user-written invariant site over a marked field's block, CERASTES_INVARIANT(s.data == NULL || cerastes_owns_block(s.data)), with an unwritten malloc'd buffer: a written site is a plain site and claims the flags (decisions 0042, 0054), so the loop entry reports uninitialized, as it does for an unmarked field (partF3/loop_written_unmarked.c); the way out is to write the buffer before the loop (the twin)
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): one uninitialized error at the loop entry (observed: partF3/loop_written_marked.c); a limitation to lift later, not a verdict about the program
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(s.data == NULL || cerastes_owns_block(s.data))  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i++;
  }
  free(s.data);
  return 0;
}
