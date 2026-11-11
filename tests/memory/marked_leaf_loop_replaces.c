// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 4 (audit S5): a marked struct leaf in a loop whose body frees the buffer and stores a freshly allocated, written one: the carried site is the user's own and the program verifies
// Pre-registered from record 0058 item 4 (audit S5, il-analyst report 6): verifies in both modes (build/m4.5d2b2d/emul3/lr2.c)
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
    if (s.data != NULL) {
      *s.data = 1;
    }
    i++;
  }
  free(s.data);
  return 0;
}
