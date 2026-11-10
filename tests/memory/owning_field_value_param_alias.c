// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: a by-value parameter with no owns_block(s.data) site passes an alias: the callee only reads it and the caller still owns the block afterwards, which it frees
// Pre-registered from record 0055 item 6 (copies and values): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int peek(struct buf s)
    CERASTES_ENSURES(r, r == s.n)
{
  return s.n;
}

int use(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  struct buf b = { NULL, 3 };
  b.data = (int *)malloc(sizeof(int));
  if (b.data == NULL) {
    return -1;
  }
  int v = peek(b);
  free(b.data);
  return v;
}
