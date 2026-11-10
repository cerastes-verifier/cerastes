// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 ("A site rooted at another parameter ... keeps its tie"): a bodiless contract with a marked first parameter, which needs no tie, and an unmarked second parameter, whose tie q->data == old(q->data) is still written; the caller allocates both structures and checks both fields stay NULL
// Pre-registered from record 0058 item 2: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch2(struct buf *p CERASTES_NOT_INVALIDATING, struct buf *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(*q) && q->data == cerastes_old(q->data));

void buf_destroy(struct buf *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns_block(p))
{
  if (p != NULL) {
    free(p->data);
    free(p);
  }
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct buf *b = (struct buf *)calloc(1, sizeof(struct buf));
  if (b == NULL) {
    return -1;
  }
  struct buf *c = (struct buf *)calloc(1, sizeof(struct buf));
  if (c == NULL) {
    buf_destroy(b);
    return -1;
  }
  touch2(b, c);
  CERASTES_CHECK(b->data == NULL && c->data == NULL);
  buf_destroy(b);
  buf_destroy(c);
  return 0;
}
