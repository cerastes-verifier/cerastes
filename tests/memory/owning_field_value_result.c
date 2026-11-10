// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: a by-value result gives the block of r.data where its contract says owns_block(r.data) (0054 item 2): the callee returns its struct local (an owner that outlives the exit), the caller's initializer takes the block into b and frees it
// Pre-registered from record 0055 item 6 (copies and values): verifies in both modes (the site is today's form; the returned owner and the initializer's leaf stores are new)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf make(int v)
    CERASTES_ENSURES(r, r.n == v && (r.data == NULL || cerastes_owns_block(r.data)))
{
  struct buf s = { NULL, v };
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = v;
  }
  return s;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct buf b = make(3);
  int n = b.n;
  free(b.data);
  return n;
}
