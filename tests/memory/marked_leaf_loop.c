// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): a marked struct leaf in a loop: a struct local whose owning field is also ref_to_uninit, with an allocated unwritten buffer, and a loop that writes only a plain member: the loop carries no closure for the marked leaf (the site has no VIR case)
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): verifies in both modes (partF3/loop_marked_synthetic.c)
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT; int n; };
int main(void)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s.n = i;
    i++;
  }
  free(s.data);
  return 0;
}
