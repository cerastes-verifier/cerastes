// expect: error uninitialized @ L28
// mode: c23, c++20
// twin: tests/memory/marked_leaf_loop.c
// Pins: 0058 item 4 (audit S5): the same marked leaf, the loop writes the buffer and the program reads it after the loop: the loop may not have run, so *s.data may have no value: uninitialized at the read
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): one uninitialized error at the read (build/m4.5d2b2d/emul3/lr3.c)
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (s.data != NULL) {
      *s.data = i;
    }
    i++;
  }
  if (s.data != NULL) {
    int y = *s.data;  // <- here
    s.n = y;
  }
  free(s.data);
  return 0;
}
