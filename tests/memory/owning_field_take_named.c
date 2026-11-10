// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 5 (N3, P4417R0 p3): a struct local's block is freed through a plain local and the field is set to NULL afterwards: the reassignment row finds the block already consumed
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), verifies in both modes (new: no base form for the named leaf)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 0 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  *s.data = 1;
  int *t = s.data;
  free(t);
  s.data = NULL;
  return 0;
}
