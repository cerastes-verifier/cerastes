// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: t = s; copies the leaves, so source and destination then hold one block (nothing is cleared); s.data = NULL then hands it to t alone: the reassignment row of s.data finds t holding the same value as an owner that outlives, and t frees it
// Pre-registered from record 0055 item 6 (copies and values): verifies in both modes (S40 (e) e_ok.c on the frozen base, by hand: verified)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf s = { NULL, 1 };
  s.data = (int *)malloc(sizeof(int));
  if (s.data == NULL) {
    return -1;
  }
  struct buf t;
  t = s;
  s.data = NULL;
  free(t.data);
  return 0;
}
