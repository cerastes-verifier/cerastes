// expect: error uninitialized @ L28
// mode: c23, c++20
// twin: tests/memory/marked_leaf_loop_replaces.c
// Pins: 0058 item 4 (audit S5): the same loop that stores the fresh buffer without writing it: an iteration may end with s.data having no value though it had one when the loop began: uninitialized at the end of the iteration
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): one uninitialized error at the closing brace of the loop body (build/m4.5d2b2d/emul3/lr.c)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    free(s.data);
    s.data = (int *)malloc(sizeof(int));
    i++;
  }  // <- here
  free(s.data);
  return 0;
}
