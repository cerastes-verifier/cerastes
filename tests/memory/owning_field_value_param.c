// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: a by-value parameter carries a block where its contract says owns_block(s.data) (0054 item 2): the callee's copy s holds it from entry and frees it; the call takes the block from the caller's b
// Pre-registered from record 0055 item 6 (copies and values): verifies in both modes (the site and the call are today's forms; the callee's leaf ghost is new)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int consume(struct buf s)
    CERASTES_REQUIRES(s.data == NULL || cerastes_owns_block(s.data))
{
  int n = s.n;
  free(s.data);
  return n;
}

int use(void)
{
  struct buf b = { NULL, 1 };
  b.data = (int *)malloc(sizeof(int));
  if (b.data == NULL) {
    return -1;
  }
  *b.data = 1;
  return consume(b);
}
