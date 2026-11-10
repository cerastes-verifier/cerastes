// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 3 ("Bodiless contracts"; 0054 item 2's tie): a bodiless borrow of a structure with an owning field says p->data == old(p->data), so nothing proves it returned another block; the caller, whose field is NULL, still knows it is NULL afterwards
// Pre-registered from record 0055 item 3: verifies in both modes (the tie holds in touch__wf)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

void touch(struct buf *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->data == cerastes_old(p->data));

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
  touch(b);
  CERASTES_CHECK(b->data == NULL);
  buf_destroy(b);
  return 0;
}
