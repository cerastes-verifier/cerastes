// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): the way out of the pinned false alarm: the buffer is written before the loop, so the written invariant site over the marked field's block holds on entry
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): verifies in both modes (build/m4.5d2b2d/emul3/lw.c)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(s.data == NULL || cerastes_owns_block(s.data))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i++;
  }
  free(s.data);
  return 0;
}
